/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <telephony/mtk_ril.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <alloca.h>
#include "atchannels.h"
#include "at_tok.h"
#include "misc.h"
#include <getopt.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <termios.h>

#include <ril_callbacks.h>
#include <cutils/properties.h>

// for BT_SAP
#include <hardware/ril/librilutils/proto/sap-api.pb.h>
#include "pb_decode.h"
#include "pb_encode.h"

#ifdef LOG_TAG
    #undef LOG_TAG
    #define LOG_TAG "RIL-STK"
#endif

#ifdef MTK_RIL
static const RIL_SOCKET_ID s_pollSimId = RIL_SOCKET_1;
#if (SIM_COUNT >= 2)
static const RIL_SOCKET_ID s_pollSimId2 = RIL_SOCKET_2;
#if (SIM_COUNT >= 3) /* Gemini plus 3 SIM*/
static const RIL_SOCKET_ID s_pollSimId3 = RIL_SOCKET_3;
#endif
#if (SIM_COUNT >= 4) /* Gemini plus 4 SIM*/
static const RIL_SOCKET_ID s_pollSimId4 = RIL_SOCKET_4;
#endif
#endif /* MTK_GEMINI */
#endif /* MTK_RIL */

#include <log/log.h>

#define STK_CHANNEL_CTX getRILChannelCtxFromToken(t)
static const struct timeval TIMEVAL_2S = {2, 0};
static const struct timeval TIMEVAL_0 = {2, 0};
static const struct timeval TIMEVAL_0S = {0, 0};
static const struct timeval TIMEVAL_200MS = {0, 20000};
static const struct timeval TIMEVAL_SMS = {0, 0};
static const struct timeval TIMEVAL_60S = {60, 0};
static sat_at_string_struct g_sms_at[RIL_SOCKET_NUM];

static sat_at_string_struct g_stk_at;
static int cpinRetry;
static RIL_SOCKET_ID rid_cpin;
static RIL_SOCKET_ID rid_menu;

/// M: Vzw SIM application test @{
static int refreshCount;
/// @}

#define EVENT_NOTIFY_BUFFER_LEN    10
static bool aIs_stk_service_running[4] = {false, false, false, false};
static bool aIs_proac_cmd_queued[4] = {false, false, false, false};
static bool aIs_pending_open_channel[4] = {false, false, false, false};
static bool aIs_event_notify_queued[4] = {false, false, false, false};
static char* pProactive_cmd[4] = {0};
static char* pStkMenu_cmd[4] = {0};
static char* pEvent_notify[SIM_COUNT][EVENT_NOTIFY_BUFFER_LEN] = {{0, 0,  0, 0 , 0, 0, 0, 0, 0, 0}};
static char* pOpenChannelCmd[4] = {0};
static char* pOpenChannelTR[4] = {0};

#define SETUP_MENU_CMD_DETAIL_CODE "81030125"

static pthread_mutex_t sFreeCmdMutex[MAX_SIM_COUNT]= {
        PTHREAD_MUTEX_INITIALIZER,
        PTHREAD_MUTEX_INITIALIZER,
        PTHREAD_MUTEX_INITIALIZER,
        PTHREAD_MUTEX_INITIALIZER};

// For SIM refresh response handling
typedef struct {
    RIL_SimRefreshResponse_v7* simRefreshRsp;
    char* efId;
} SimRefreshResponse;

int getSimIdfromSocketId(RIL_SOCKET_ID rid);
int getSimIdfromToken(RIL_Token t);
int checkEventNotifyFreeBuffer(RIL_SOCKET_ID rid);
// for BT_SAP
void sendStkBtSapResponseComplete(RIL_Token t, RIL_Errno ret, MsgId msgId, void *data);

void requestStkGetProfile (void *data, size_t datalen, RIL_Token t) {
    RIL_STK_UNUSED_PARM(data);
    RIL_STK_UNUSED_PARM(datalen);
    RIL_STK_UNUSED_PARM(t);
}

void requestStkSetProfile (void *data, size_t datalen, RIL_Token t) {
    RIL_STK_UNUSED_PARM(data);
    RIL_STK_UNUSED_PARM(datalen);
    RIL_STK_UNUSED_PARM(t);
}

void setStkFlagByTk(RIL_Token t, bool flag, bool* source)
{
    if (NULL == source) {
        LOGD("setStkFlagByTk source is null.");
        return;
    }

    if (RIL_CHANNEL_SET4_OFFSET <= RIL_queryMyChannelId(t)) {
        *(source + 3) = flag;
    } else if (RIL_CHANNEL_SET3_OFFSET <= RIL_queryMyChannelId(t)) {
        *(source + 2) = flag;
    } else if (RIL_CHANNEL_OFFSET <= RIL_queryMyChannelId(t)) {
        *(source + 1) = flag;
    } else {
        *(source) = flag;
    }

    LOGD("setStkFlagByTk[%d][%d][%d][%d].",
        *(source), *(source + 1), *(source + 2), *(source + 3));
}

void setStkFlagByRid(RIL_SOCKET_ID rid, bool flag, bool* source) {
    if (NULL == source) {
        LOGD("setStkFlagByRid source is null.");
        return;
    }

    if (RIL_SOCKET_4 == rid) {
        *(source + 3) = flag;
    } else if (RIL_SOCKET_3 == rid) {
        *(source + 2) = flag;
    } else if (RIL_SOCKET_2 == rid) {
        *(source + 1) = flag;
    } else {
        *source = flag;
    }

    LOGD("setStkFlagByRid[%d][%d][%d][%d].",
        *(source), *(source + 1), *(source + 2), *(source + 3));
}

bool getStkFlagByTk(RIL_Token t, bool* source) {
    if (RIL_CHANNEL_SET4_OFFSET <= RIL_queryMyChannelId(t)) {
        return *(source + 3);
    } else if (RIL_CHANNEL_SET3_OFFSET <= RIL_queryMyChannelId(t)) {
        return *(source + 2);
    } else if (RIL_CHANNEL_OFFSET <= RIL_queryMyChannelId(t)) {
        return *(source + 1);
    } else {
        return *(source);
    }
}

bool getStkFlagByRid(RIL_SOCKET_ID rid, bool* source) {
    if (RIL_SOCKET_4 == rid) {
        return *(source + 3);
    } else if (RIL_SOCKET_3 == rid) {
        return *(source + 2);
    } else if (RIL_SOCKET_2 == rid) {
        return *(source + 1);
    } else {
        return *source;
    }
}

char* getStkCachedDataByRid(RIL_SOCKET_ID rid, char** source) {
    if (RIL_SOCKET_4 == rid) {
        return *(source + 3);
    } else if (RIL_SOCKET_3 == rid) {
        return *(source + 2);
    } else if (RIL_SOCKET_2 == rid) {
        return *(source + 1);
    } else {
        return *source;
    }
}
char* getStkCachedData(RIL_Token t, char** source) {

    if (RIL_CHANNEL_SET4_OFFSET <= RIL_queryMyChannelId(t)) {
        return *(source + 3);
    } else if (RIL_CHANNEL_SET3_OFFSET <= RIL_queryMyChannelId(t)) {
        return *(source + 2);
    } else if (RIL_CHANNEL_OFFSET <= RIL_queryMyChannelId(t)) {
        return *(source + 1);
    } else {
        return *source;
    }
}

void setStkCachedData(RIL_SOCKET_ID rid, char** source, char* pCmd) {
    if (NULL == source) {
        LOGD("setStkCachedData source is null.");
        return;
    }

    if (RIL_SOCKET_4 == rid) {
        *(source + 3) = pCmd;
    } else if (RIL_SOCKET_3 == rid) {
        *(source + 2) = pCmd;
    } else if (RIL_SOCKET_2 == rid) {
        *(source + 1) = pCmd;
    } else {
        *source = pCmd;
    }
    LOGD("setStkCachedData rid:%d, [%p][%p][%p][%p].",
        rid, *(source), *(source + 1), *(source + 2), *(source + 3));
}

void setStkServiceRunningFlag(RIL_SOCKET_ID rid, bool flag) {
    setStkFlagByRid(rid, flag, aIs_stk_service_running);
}

void resetAllStkServiceRunningFlag() {
    LOGD("resetAllStkServiceRunningFlag");
    int i;
    for (i = 0; i < SIM_COUNT; i++) {
        setStkServiceRunningFlag(RIL_SOCKET_1 + i, false);
    }
}

void resetAllStkServiceRunningFlagDelayed(RIL_SOCKET_ID rid) {
    LOGD("resetAllStkServiceRunningFlagDelayed");
    RIL_requestProxyTimedCallback(resetAllStkServiceRunningFlag, NULL, &TIMEVAL_2S,
        getRILChannelId(RIL_STK, rid), "resetAllStkServiceRunningFlag");
}

bool getStkProactiveCmdQueuedFlag(RIL_Token t) {
    LOGD("getStkProactiveCmdQueuedFlag[%d][%d][%d][%d].", aIs_proac_cmd_queued[0],
            aIs_proac_cmd_queued[1], aIs_proac_cmd_queued[2], aIs_proac_cmd_queued[3]);

    if (RIL_CHANNEL_SET4_OFFSET <= RIL_queryMyChannelId(t)) {
        return aIs_proac_cmd_queued[3];
    } else if (RIL_CHANNEL_SET3_OFFSET <= RIL_queryMyChannelId(t)) {
        return aIs_proac_cmd_queued[2];
    } else if (RIL_CHANNEL_OFFSET <= RIL_queryMyChannelId(t)) {
        return aIs_proac_cmd_queued[1];
    } else {
        return aIs_proac_cmd_queued[0];
    }
}

void sendTerminalResponseForSimPlugOut() {
    char* cmd = NULL;
    char* data = "810301250082028281830100";
    ATResponse *p_response = NULL;
    int err;

    asprintf(&cmd, "AT+STKTR=\"%s\"", data);

    LOGE("sendTerminalResponseForSimPlugOut: rid:%d, cmd:%s", rid_menu, cmd);
    err = at_send_command(cmd, &p_response, getChannelCtxbyProxy());
    free(cmd);

    at_response_free(p_response);
}

void freeProactiveCmd(int simId) {
    pthread_mutex_lock(&sFreeCmdMutex[simId]);
    if (NULL != pProactive_cmd[simId]) {
        free(pProactive_cmd[simId]);
        pProactive_cmd[simId] = NULL;
    }
    pthread_mutex_unlock(&sFreeCmdMutex[simId]);
}

void freeStkQueuedProactivCmd(RIL_Token t) {
    int simId = getSimIdfromToken(t);

    if (simId >= SIM_COUNT) {
        LOGE("freeStkQueuedProactivCmd: Invalid simId:%d !!!", simId);
        return;
    }

    freeProactiveCmd(simId);
}

void freeStkQueuedProactivCmdByRid(RIL_SOCKET_ID rid) {
    int simId = getSimIdfromSocketId(rid);
    rid_menu= rid;

    if (NULL != pProactive_cmd[simId]) {
        LOGD("freeStkQueuedProactivCmdByRid, rilId = %d", rid_menu);
        RIL_requestProxyTimedCallback(sendTerminalResponseForSimPlugOut, NULL, &TIMEVAL_0S,
                                          getRILChannelId(RIL_STK, rid), "sendTerminalResponseForSimPlugOut");
    }
    freeProactiveCmd(simId);
}

void freeStkCacheMenuByRid(RIL_SOCKET_ID rid) {
    int simId = getSimIdfromSocketId(rid);
    LOGD("freeStkCacheMenuByRid: [%d].", rid);
    if (NULL != pStkMenu_cmd[simId]) {
        free(pStkMenu_cmd[simId]);
        pStkMenu_cmd[simId] = NULL;
    }
}

bool getStkEventNotifyQueuedFlag(RIL_Token t) {
    LOGD("getStkEventNotifyQueuedFlag[%d][%d][%d][%d].", aIs_event_notify_queued[0],
            aIs_event_notify_queued[1], aIs_event_notify_queued[2], aIs_event_notify_queued[3]);
    if (RIL_CHANNEL_SET4_OFFSET <= RIL_queryMyChannelId(t)) {
        return aIs_event_notify_queued[3];
    } else if (RIL_CHANNEL_SET3_OFFSET <= RIL_queryMyChannelId(t)) {
        return aIs_event_notify_queued[2];
    } else if (RIL_CHANNEL_OFFSET <= RIL_queryMyChannelId(t)) {
        return aIs_event_notify_queued[1];
    } else {
        return aIs_event_notify_queued[0];
    }
}

char* getStkQueuedEventNotify(RIL_Token t, int index) {
    int simId = getSimIdfromToken(t);

    if (simId >= SIM_COUNT) {
        LOGE("getStkQueuedEventNotify: Invalid simId:%d !!!", simId);
        return NULL;
    }

    if (index >= EVENT_NOTIFY_BUFFER_LEN) {
        LOGE("getStkQueuedEventNotify: Invalid index:%d !!!", index);
        return NULL;
    }

    return pEvent_notify[simId][index];
}

void freeStkQueuedEventNotify(RIL_Token t, int index) {
    int simId = getSimIdfromToken(t);

    if (simId >= SIM_COUNT) {
        LOGE("freeStkQueuedEventNotify: Invalid simId:%d !!!", simId);
        return;
    }

    if (index >= EVENT_NOTIFY_BUFFER_LEN) {
        LOGE("freeStkQueuedEventNotify: Invalid index:%d !!!", index);
        return;
    }

    if (NULL != pEvent_notify[simId][index]) {
        free(pEvent_notify[simId][index]);
        pEvent_notify[simId][index] = NULL;
    }
}

void setStkQueuedEventNotifyWithRid(RIL_SOCKET_ID rid, char* pCmd) {
    int i = 0;
    int simId = getSimIdfromSocketId(rid);

    if (simId >= SIM_COUNT) {
        LOGE("setStkQueuedEventNotifyWithRid: Invalid simId:%d !!!", simId);
        return;
    }

    for (i = 0 ; i < EVENT_NOTIFY_BUFFER_LEN ; i++) {
        if (NULL == pEvent_notify[simId][i]) {
            pEvent_notify[simId][i] = pCmd;
            break;
        }
    }

    if (EVENT_NOTIFY_BUFFER_LEN == i) {
        LOGE("setStkQueuedEventNotifyWithRid: No free buffer !!!");
    } else {
        //LOGD("setStkQueuedEventNotifyWithRid[socketId:%d][index:%d][%p][%s]",
        //    rid, i, pCmd, pCmd);
    }
}

void onStkAtSendFromUrc()
{
    int ret;
    int session_id = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    ATResponse *p_response = NULL;

    property_get("vendor.ril.rc.session.id1", property_value, "-1");
    session_id = atoi(property_value);

    switch(g_stk_at.cmd_type) {
        case CMD_SETUP_CALL:
            if (g_stk_at.cmd_res == 50) {
                at_send_command("AT+STKCALL=50", NULL, getChannelCtxbyProxy());
            }
            break;
        case CMD_DTMF:
            // at_send_command("AT+STKDTMF=0", NULL, getChannelCtxbyProxy());
            if(inCallNumber != 0) {
                ret = at_send_command("AT+STKDTMF=0", &p_response, getChannelCtxbyProxy());
                if (ret < 0 || p_response->success == 0) {
                    at_send_command("AT+STKDTMF=32,9", NULL, getChannelCtxbyProxy());
                }
            } else {
                at_send_command("AT+STKDTMF=32,7", NULL, getChannelCtxbyProxy());
            }
            break;
        case CMD_SEND_SMS:
            at_send_command("AT+STKSMS=0", NULL, getChannelCtxbyProxy());
            break;
        case CMD_SEND_SS:
            if (g_stk_at.cmd_res == 50) {
                at_send_command("AT+STKSS=50", NULL, getChannelCtxbyProxy());
            } else if (g_stk_at.cmd_res == 0) {
                ret = at_send_command("AT+STKSS=0", &p_response, getChannelCtxbyProxy());
                if (ret < 0 || p_response->success == 0) {
                    at_send_command("AT+STKSS=32,3", NULL, getChannelCtxbyProxy());
                }
            }
            break;
        case CMD_SEND_USSD:
            if (session_id != -1) {
                LOGE("onStkAtSendFromUrc: During sim switch and ignore to handle CMD_SEND_USSD!");
                break;
            }
            if (g_stk_at.cmd_res == 50) {
                at_send_command("AT+STKUSSD=50", NULL, getChannelCtxbyProxy());
            } else if (g_stk_at.cmd_res == 0) {
                ret = at_send_command("AT+STKUSSD=0", &p_response, getChannelCtxbyProxy());
                if (ret < 0 || p_response->success == 0) {
                    at_send_command("AT+STKUSSD=32,8", NULL, getChannelCtxbyProxy());
                }
            }
            break;
        default:
            break;
    }

    if (p_response != NULL) {
        at_response_free(p_response);
        p_response = NULL;
    }
}

void queryCpinStatus(void *param) {
    int err;
    ATResponse *p_response = NULL;
    char *cpinLine;
    char *cpinResult;
    int success = 0;

    if (NULL == param) {
        LOGE("queryCpinStatus: param is NULL !!!");
        return;
    }

    err = at_send_command_singleline("AT+CPIN?", "+CPIN:", &p_response,
        getChannelCtxbyProxy());
    if (err != 0) {
        goto retryQuery;
    }

    if (p_response->success == 0) {
        int cmeError = at_get_cme_error(p_response);
        LOGE("queryCpinStatus error, cmeError = %d", cmeError);
        goto retryQuery;
    }

    /* CPIN? has succeeded, now look at the result */
    cpinLine = p_response->p_intermediates->line;
    //LOGD("queryCpinStatus: cpinLine1 = %s", cpinLine);
    err = at_tok_start(&cpinLine);
    //LOGD("queryCpinStatus: cpinLine2 = %s", cpinLine);

    if (err < 0) {
        goto retryQuery;
    }

    err = at_tok_nextstr(&cpinLine, &cpinResult);
    //LOGD("queryCpinStatus: cpinLine3 = %s", cpinLine);

    if (err < 0) {
        goto retryQuery;
    }

    /* All what we wait is CPIN:READY */
    if (0 == strcmp(cpinResult, "READY"))  {
        success = 1;
        LOGD("queryCpinStatus: success");
        // goto done;
    }

retryQuery:
    cpinRetry++;
    LOGD("queryCpinStatus: success = %d, cpinRetry:%d", success, cpinRetry);
    if (!success && cpinRetry < 15) {
        RIL_requestProxyTimedCallback(queryCpinStatus, param, &TIMEVAL_200MS,
        getRILChannelId(RIL_STK, rid_cpin), "queryCpinStatus");
    } else {
        SimRefreshResponse *simRefreshResponse = (SimRefreshResponse*)param;
        char* pAid;
        char* pEfIdString;
        int files_num = 0;
        int i = 0;
        int readIdx = 0;
        char temp_str[5] = { 0 }; // for read EFID
        int efId = 0;
        pEfIdString = simRefreshResponse->efId;
        if (pEfIdString) {
            if (NULL != pEfIdString && 4 <= strlen(pEfIdString)) {
                files_num = strlen(pEfIdString) / 4;
            }
            // Try to read each EFID and send the URC for SIM REFRESH
            for (i = 0; i < files_num; i++) {
                memcpy(temp_str, pEfIdString + readIdx, 4);
                temp_str[4] = '\0';
                efId = strtol(temp_str, NULL, 16);
                simRefreshResponse->simRefreshRsp->ef_id = efId;
                LOGD("queryCpinStatus: efId = %X, file numbers = %d", efId, files_num);
                RIL_onUnsolicitedResponse( RIL_UNSOL_SIM_REFRESH, simRefreshResponse->simRefreshRsp,
                        sizeof(RIL_SimRefreshResponse_v7), rid_cpin);
                readIdx += 4; // go to next EFID
            }
        } else {
            RIL_onUnsolicitedResponse( RIL_UNSOL_SIM_REFRESH, simRefreshResponse->simRefreshRsp,
                    sizeof(RIL_SimRefreshResponse_v7), rid_cpin);
        }
        if (simRefreshResponse->simRefreshRsp->aid != NULL) {
            free(simRefreshResponse->simRefreshRsp->aid);
        }
        free(simRefreshResponse->simRefreshRsp);
        if (simRefreshResponse->efId != NULL) {
            free(simRefreshResponse->efId);
        }
        free(simRefreshResponse);
        simRefreshResponse = NULL;
    }

    at_response_free(p_response);
    p_response = NULL;
    cpinResult = NULL;
    return;
}

void onStkAtSendSmsFromUrc(void *param)
{
    RIL_SOCKET_ID rid = *((RIL_SOCKET_ID *)param);

    LOGD("onStkAtSendSmsFromUrc: rid:%d ", rid);
    at_send_command("AT+STKSMS=0", NULL, getChannelCtxbyProxy());
}

void StkSendRequestComplete(int err, ATResponse *p_response, RIL_Token t)
{
    if (err < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
}

void requestReportStkServiceIsRunning(void *data, size_t datalen, RIL_Token t)
{
    RIL_STK_UNUSED_PARM(data);
    RIL_STK_UNUSED_PARM(datalen);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(STK_CHANNEL_CTX);
    if (isSimInserted(rid)) {
        setStkFlagByTk(t, true, aIs_stk_service_running);
    } else {
        LOGE("STK service is running but sim plug out!");
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        return;
    }

    if (true == getStkProactiveCmdQueuedFlag(t)) {
        LOGD("STK service is running: is_proac_cmd_queued: true");
        char *cmd = (char *)getStkCachedData(t, pProactive_cmd);
        if (NULL != cmd) {
            LOGD("Proactive Cmd:[%zu][%s]", strlen(cmd), cmd);
            MTK_UNSOL_STK_PROACTIVE_COMMAND(cmd, STK_CHANNEL_CTX);
            freeStkQueuedProactivCmd(t);
        }
        setStkFlagByTk(t, false, aIs_proac_cmd_queued);
    } else {
        LOGD("STK service is running: is_proac_cmd_queued: false");
        char *cmd = (char *)getStkCachedData(t, pStkMenu_cmd);
        if (NULL != cmd) {
            LOGD("Stk menu:[%zu][%s][%p]", strlen(cmd), cmd, cmd);

            RIL_onUnsolicitedResponse (
                RIL_UNSOL_STK_PROACTIVE_COMMAND, cmd, strlen(cmd),rid);
            freeStkCacheMenuByRid(rid);
        }
    }

    if (true == getStkEventNotifyQueuedFlag(t)) {
        LOGD("STK service is running: is_event_notify_queued: true");
        int index = 0;
        while (index < EVENT_NOTIFY_BUFFER_LEN &&
               NULL != getStkQueuedEventNotify(t, index)) {
            char *cmd = (char *)getStkQueuedEventNotify(t, index);
            if (NULL != cmd) {
                //Remove for sensitive log task
                //LOGD("Event Notify:[index:%d][%d][%s]", index, strlen(cmd), cmd);
                MTK_UNSOL_STK_EVENT_NOTIFY(cmd, STK_CHANNEL_CTX);
                freeStkQueuedEventNotify(t, index);
            }
            index++;
        }
        setStkFlagByTk(t, false, aIs_event_notify_queued);
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

void requestStkSendEnvelopeCommandWithStatus (void *data, size_t datalen, RIL_Token t)
{
    char* cmd = NULL;
    ATResponse *p_response = NULL;
    int err;
    char *line = NULL;
    RIL_SIM_IO_Response sr;
    ATLine *p_cur = NULL;
    bool headIntermediate = true;

    RIL_STK_UNUSED_PARM(datalen);

    memset(&sr, 0, sizeof(sr));

    RIL_SOCKET_ID rid = getRILIdByChannelCtx(STK_CHANNEL_CTX);
    if (!isSimInserted(rid)) {
        RIL_onRequestComplete(t, RIL_E_SIM_ABSENT, &sr, sizeof(sr));
        return;
    }
    data = (data == NULL) ? ((void *)("")) : data;
    asprintf(&cmd, "AT+CUSATE=\"%s\"", (char *)data);

    err = at_send_command_raw(cmd, &p_response, STK_CHANNEL_CTX);
    if (err < 0 || p_response->success == 0) {
        goto error;
    }

    for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
        line = p_cur->line;
        if (NULL == line) {
            LOGE("requestStkSendEnvelopeCommand ok but no intermediates.");
            goto done;
        }

        if (headIntermediate) {
            headIntermediate = false;
            LOGD("CUSATE,[%s]", line);
            //sub_line = strstr(line, "+CUSATE2:");
            err = at_tok_start(&line);
            if (err < 0) {
                LOGE("get +CUSATE: error.");
                goto error;
            }
            err = at_tok_nextstr(&line, &(sr.simResponse));
            if (err < 0) {
                LOGE("response data is null.");
                goto error;
            }
        } else {
            /*skip <busy> field, just deal with sw1 sw2.*/
            LOGD("CUSATE2,[%s]", line);
            err = at_tok_start(&line);
            if (err < 0) {
                LOGE("get +CUSATE2: error.");
                goto error;
            }
            err = at_tok_nextint(&line, &(sr.sw1));
            if (err < 0) goto error;
            err = at_tok_nextint(&line, &(sr.sw2));
            if (err < 0) goto error;
            LOGD("requestStkSendEnvelopeCommand sw: %02x, %02x", sr.sw1, sr.sw2);
        }
    }
    goto done;

error:
    RIL_onRequestComplete(t, RIL_E_INVALID_ARGUMENTS, NULL, 0);
    free(cmd);
    at_response_free(p_response);
    return;
done:
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &sr, sizeof(sr));
    free(cmd);
    at_response_free(p_response);
    return;
}

int getElementFromCtlvStructure(char *cmd) {
    char temp[3] = {0};
    int cmdType = 0;

    strncpy(temp, cmd, 2);
    cmdType = strtoul(temp, NULL, 16);
    cmdType = 0xFF & cmdType;

    return cmdType;
}

void parseStkEnvelopeCommandType(char* cmd, int* cmdType) {
    int typePos = 0;

    // As TS 102 223, the lenth of event download message is more than 9 Bytes
    if (cmd == NULL || strlen(cmd) < 18) {
        LOGE("Envelop cmd length error!");
        return;
    }

    // Currently only check BER-TLV tag and D6 means this is an event download message
    if ((cmd[0] == 'D' || cmd[0] == 'd') && cmd[1] == '6') {
        if (cmd[2] <= '7') {
            // As TS 102 223, the length field will take 1 byte if the first value is less than 8
            // so the command type will start from the 5th byte.
            typePos = 8;
        } else {
            typePos = 10;
        }
        *cmdType = getElementFromCtlvStructure(&cmd[typePos]);
        LOGD("parseEnvelopeType event type:%d", *cmdType);
    } else {
        // For other envelope command, just return
        LOGE("Not Event download cmd and return!");
    }
}

void parseStkTerminalResponseType(char* cmd, int* cmdType, int* qualifiler) {
    int typePos = 0;

    // As TS 102 223, the lenth of command details structure is 5 Bytes
    if (cmd == NULL || strlen(cmd) < 10) {
        LOGE("Terminal response length error!");
        return;
    }

    // Check COMPREHENSION-TLV tag and 01/81 means this is a command details structure
    if ((cmd[0] == '0' || cmd[0] == '8') && cmd[1] == '1') {
        // As TS 102 223, the length value in command details is always 3 Bytes
        // so the command type will start from the 4th byte.
        typePos = 6;
        // Get the value of command type and command qualifier
        *cmdType = getElementFromCtlvStructure(&cmd[typePos]);
        *qualifiler = getElementFromCtlvStructure(&cmd[typePos+2]);
        LOGD("parseStkTerminalResponseType: cmd type:%d,cmd qualifier: %d", *cmdType, *qualifiler);
    } else {
        // For other command, just return
        LOGE("Not terminal response and just return!");
    }
}

bool isIgnoredTerminalResponse(char* cmd) {
    bool bIgnored = false;
    int cmdType = -1;
    int qualifiler = -1;

    parseStkTerminalResponseType(cmd, &cmdType, &qualifiler);
    switch(cmdType) {
        case CMD_MORE_TIME:
        case CMD_POLL_INTERVAL:
        case CMD_POLLING_OFF:
        case CMD_SETUP_EVENT_LIST:
        case CMD_SEND_SS:
        case CMD_SEND_SMS:
        case CMD_TIMER_MANAGER:
            bIgnored = true;
            break;
        case CMD_PROVIDE_LOCAL_INFO:
            if ((0x08 == qualifiler) || (0x09 == qualifiler) || (0x10 == qualifiler))
                bIgnored = true;
            break;
        default:
            break;
    }
    LOGD("Cmd type:%d need be ignored: %d", cmdType, bIgnored);
    return bIgnored;
}

bool isIgnoredEnvelopeCommand(char* cmd) {
    bool bIgnored = false;
    int eventType = -1;

    parseStkEnvelopeCommandType(cmd, &eventType);
    switch(eventType) {
        case EVENT_MT_CALL:
        case EVENT_CALL_CONNECTED:
        case EVENT_CALL_DISCONNECTED:
        case EVENT_LOCATION_STATUS:
        case EVENT_ACCESS_TECHNOLOGY_CHANGE:
        case EVENT_NETWORK_SEARCH_MODE_CHANGE:
        case EVENT_NETWORK_REJECTION:
            bIgnored = true;
            break;
        default:
            break;
    }
    LOGD("Event type:%d need be ignored: %d", eventType, bIgnored);
    return bIgnored;
}

void requestStkSendEnvelopeCommand (void *data, size_t datalen, RIL_Token t)
{
    char* cmd = NULL;
    ATResponse *p_response = NULL;
    int err;
    bool isIgnoredCmd = false;

    RIL_STK_UNUSED_PARM(datalen);

    //Filter and ignore the command which has handled by modem
    isIgnoredCmd = isIgnoredEnvelopeCommand(data);
    if (isIgnoredCmd) {
        LOGD("requestStkSendEnvelopeCommand: ignore the cmd which is handled by modem");
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        return;
    }
    asprintf(&cmd, "AT+STKENV=\"%s\"", (char *)data);

    err = at_send_command(cmd, &p_response, STK_CHANNEL_CTX);
    free(cmd);
    // To fix VTS fail problem
     char *response = "";
    if (err < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, response, strlen(response));
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, response, strlen(response));
    }

    at_response_free(p_response);

}

void requestStkSendTerminalResponse (void *data, size_t datalen, RIL_Token t)
{
    char* cmd = NULL;
    ATResponse *p_response = NULL;
    int err;
    bool isIgnoredCmd = false;

    RIL_STK_UNUSED_PARM(datalen);

    //Filter and ignore the command which has handled by modem
    isIgnoredCmd = isIgnoredTerminalResponse(data);
    if (isIgnoredCmd) {
        LOGD("requestStkSendTerminalResponse: ignore the cmd which is handled by modem");
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        return;
    }
    data = (data == NULL) ? ((void *)("")) : data;
    asprintf(&cmd, "AT+STKTR=\"%s\"", (char *)data);

    if (true == getStkFlagByTk(t, aIs_pending_open_channel)) {
        setStkFlagByTk(t, false, aIs_pending_open_channel);
    }
    err = at_send_command(cmd, &p_response, STK_CHANNEL_CTX);
    free(cmd);

    //if (strStartsWith((char*)data, SETUP_MENU_CMD_DETAIL_CODE)) {
    //    freeStkCacheMenuByRid(getRILIdByChannelCtx(STK_CHANNEL_CTX));
    //}

    StkSendRequestComplete(err, p_response, t);

    at_response_free(p_response);
}

void requestStkHandleCallSetupRequestedFromSim (void *data, size_t datalen, RIL_Token t)
{
    char* cmd;
    ATResponse *p_response = NULL;
    int err = 0, user_confirm = 0, addtional_info = 0;

    RIL_STK_UNUSED_PARM(datalen);
    char *temp_str = NULL;
    int temp_int = 0;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(STK_CHANNEL_CTX);
    LOGD("requestStkHandleCallSetupRequestedFromSim");
    if (true == getStkFlagByTk(t, aIs_pending_open_channel)) {
        setStkFlagByTk(t, false, aIs_pending_open_channel);
        if (((int *)data)[0] == 1) {
            //redirect OPEN CHANNEL to BipService.
            cmd = getStkCachedData(t, pOpenChannelCmd);
            if (NULL == cmd) {
                LOGD("cmd is null.");
                return;
            }
            LOGD("cmd: %s.", cmd);
            RIL_onUnsolicitedResponse (
                RIL_UNSOL_STK_BIP_PROACTIVE_COMMAND,
                cmd, strlen(cmd),
                rid);
            return;
        } else {
            //response TR of OPEN CAHNNEL to SIM directlly.
            char* cachedTR = getStkCachedData(t, pOpenChannelTR);
            LOGD("cachedTR: %s.", cachedTR);
            requestStkSendTerminalResponse(cachedTR, strlen(cachedTR), t);
            return;
        }
   }

    if(((int *)data)[0] == 1) {
        if (getRadioState(rid) == RADIO_STATE_OFF ||
                getRadioState(rid) == RADIO_STATE_UNAVAILABLE) {
            LOGD("radio state is off or unavailable, no send AT+STKCALL = 0");
            user_confirm = 32;
            addtional_info = 4;
        } else {
            user_confirm = 0;
        }
    } else if(((int *)data)[0] == 32) { //ME currently unable to process
        user_confirm = 32;
        addtional_info = 2;
    } else if(((int *)data)[0] == 33) { //NW currently unable to process
        user_confirm = 33;
        addtional_info = 0x9d;
    } else if(((int *)data)[0] == 0) {
        user_confirm = 34;
    } else {
        assert(0);
    }

    if( addtional_info == 0) {
        asprintf(&cmd, "AT+STKCALL=%d", user_confirm);
    } else {
        asprintf(&cmd, "AT+STKCALL=%d, %d", user_confirm, addtional_info);
    }

    err = at_send_command(cmd, &p_response, STK_CHANNEL_CTX);
    free(cmd);

    StkSendRequestComplete(err, p_response, t);

    at_response_free(p_response);
}

void onStkSessionEnd(char* urc, RILChannelCtx* p_channel)
{
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);

    RIL_STK_UNUSED_PARM(urc);

    RIL_onUnsolicitedResponse (
        RIL_UNSOL_STK_SESSION_END,
        NULL, 0,
        rid);
    return;
}


#define NUM_STK_CALL_CTRL 3
void onStkCallControl(char* urc, RILChannelCtx* p_channel)
{
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);

    int err, i = 0;
    char *p_cur = (char *)urc;
    char *responseStr[NUM_STK_CALL_CTRL] = {0};
    //LOGD("onStkCallControl URC = %s", urc);

    /**
     * +STKCTRL: <mmi_info_type>,"<alphs_id>","<address>or<ss string>or<ussd string>"
     **/
    err = at_tok_start(&p_cur);
    if (err < 0) goto error;

    for (i = 0; i < NUM_STK_CALL_CTRL; i++) {
        err = at_tok_nextstr(&p_cur, &(responseStr[i]));
        if (err < 0) {
            //responseStr[i] = 0xFF;
            LOGE("There is something wrong with item [%d]",i);
        }//goto error;
    }
    RIL_onUnsolicitedResponse(
            RIL_UNSOL_STK_CC_ALPHA_NOTIFY,
            responseStr, sizeof(responseStr),
            rid);
    return;
error:
    LOGE("There is something wrong with the +STKCTRL");
}

int checkStkCmdDisplay(char *cmd_str)
{
    int is_alpha_id_existed = 0;
    int is_icon_existed_not_self_explanatory = 0;
    int index = 0;
    int cmd_length = 0;
    char temp_str[3] = {0};
    char *end;

    cmd_length = strlen(cmd_str);

    while (cmd_length > index) {
        if (cmd_str[index + 1] == '5' && (cmd_str[index] == '0' || cmd_str[index] == '8') ) {
            index += 2;
            if (cmd_str[index] != '0' || cmd_str[index + 1] != '0' ) {
                is_alpha_id_existed = 1;
            }
            if (cmd_str[index] <= '7') {
                memcpy(temp_str, &(cmd_str[index]), 2);
                index += (strtoul(temp_str, &end, 16) + 1) * 2;
            } else {
                memcpy(temp_str, &(cmd_str[index + 2]), 2);
                index += (strtoul(temp_str, &end, 16) + 2) * 2;
            }
        } else if((cmd_str[index + 1] == 'E' || cmd_str[index + 1] == 'e') && (cmd_str[index] == '1' || cmd_str[index] == '9')) {
            int icon_qualifier = 0;

            index += 4;
            memset(temp_str, 0, 3);
            memcpy(temp_str, &(cmd_str[index + 1]), 1);
            icon_qualifier = strtoul(temp_str, &end, 16);

            if((icon_qualifier & 0x01) == 0x01) {
                if(is_alpha_id_existed == 0) {
                    return 1;
                }
            }
            index += 4;
        } else {
            index += 2;
            if (cmd_str[index] <= '7') {
                memcpy(temp_str, &(cmd_str[index]), 2);
                index += (strtoul(temp_str, &end, 16) + 1) * 2;
            } else {
                memcpy(temp_str, &(cmd_str[index + 2]), 2);
                index += (strtoul(temp_str, &end, 16) + 2) * 2;
            }
        }
    }
    return 0;
}

int checkStkCommandType(char *cmd_str)
{
    char temp_str[3] = {0};
    char *end;
    int cmd_type = 0;

    memcpy(temp_str, cmd_str, 2);

    cmd_type = strtoul(temp_str, &end, 16);
    cmd_type = 0x7F & cmd_type;

    return cmd_type;
}

char* decodeStkRefreshFileChange(char *str, int **cmd, int *cmd_length)
{
    int str_length = 0, file_num = 0, offset = 20, cmdoffset = 0;
    /*offset 20 including cmd_detail tlv: 10, device id tlv:8, file list tag:2*/
    char temp_str[5] = {0};
    char *end;
    str_length = strlen(str);
    char *efId_str;
    int file_idx = 0;

    if(str[offset] <= '7') { //file list length: if length < 7F it will use 2 bytes else it will use 4 bytes
        offset += 2;
    } else {
        offset += 4;
    }
    memcpy(temp_str, str + offset, 2); //copy number of files in file list to temp_str
    offset += 2;

    file_num = strtoul(temp_str, &end, 16);
    efId_str = (char*)calloc(1, (file_num * (sizeof(char) * 4)) + 1);
    assert(efId_str != NULL);
#ifndef MTK_WIFI_CALLING_RIL_SUPPORT
    *cmd_length = (file_num + 1) * sizeof(int);
    *cmd = (int*)calloc(1, *cmd_length);
    assert(*cmd != NULL);
#else
    *cmd_length = (file_num + 2) * sizeof(int);
    *cmd = (int*)calloc(1, *cmd_length);
    assert(*cmd != NULL);
    cmdoffset++;
#endif

    *(*cmd + cmdoffset) = SIM_FILE_UPDATE;

    cmdoffset++;
    LOGD("decodeStkRefreshFileChange file_num = %d", file_num);

    while( offset < str_length && file_num > file_idx) {
        if(((str[offset] == '6') || (str[offset] == '2') || (str[offset] == '4'))
           && ((str[offset + 1] == 'F') || (str[offset+1] == 'f'))) {
            memcpy(temp_str, str + offset, 4); //copy EFID to temo_str
            strncat(efId_str, temp_str, 4);
            *(*cmd + cmdoffset) = strtoul(temp_str, &end, 16);

            cmdoffset++;
            file_idx++;
        }
        offset += 4;
    }
    return efId_str;
}

extern int rild_sms_hexCharToDecInt(char *hex, int length);

int decodeStkRefreshAid(char *urc, char **paid)
{
    int offset = 18; //cmd_details & device identifies
    int refresh_length = strlen(urc) / 2;
    int files_length;
    int files_offset;
    int temp;   // For reading data

    if (offset >= refresh_length) {
        *paid = NULL;
        return 0;
    }

    temp = rild_sms_hexCharToDecInt(&urc[offset], 2);
    offset += 2;
    if (temp == 0x12 || temp == 0x92) { //file list tag
        temp = rild_sms_hexCharToDecInt(&urc[offset], 2);   // Length of bytes following (bytes)
        if (temp < 0x7F) {
            offset += (2 + temp * 2);
        } else {
            offset += 2;
            temp = rild_sms_hexCharToDecInt(&urc[offset], 2);
            offset += (2 + temp * 2);
        }
        temp = rild_sms_hexCharToDecInt(&urc[offset], 2);   // Number of files
        offset += 2;
    }

    LOGD("decodeStkRefreshAid temp = %02x, offset = %d", temp, offset);

    if (temp == 0x2F || temp == 0xAF) { // aid tag
        temp = rild_sms_hexCharToDecInt(&urc[offset], 2);  // read the length of AID
        offset += 2;
        *paid = &urc[offset];
        return temp * 2;
    }

    return -1;
}

/// M: Vzw SIM application test @{
void resetRefreshCount(void *param) {
    RIL_STK_UNUSED_PARM(param);
    refreshCount = 0;
    LOGD("resetRefreshCount: refreshCount = %d", refreshCount);
    return;
}
/// @}

void onSimRefresh(char* urc, RILChannelCtx * p_channel)
{
    int *cmd = NULL;
    int cmd_length = 0;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    rid_cpin = rid;
    RIL_SimRefreshResponse_v7 simRefreshRspV7;
    RIL_SimRefreshResponse_v7 *pSimRefreshRsp = NULL;
    int refreshRspMemLen = 0;

    int aid_len = 0;
    char *aid = NULL;

    char *efId_str = NULL;
    int i = 0;
    int files_num = 0;
    int readIdx = 0;
    int efId = 0;
    char temp_str[5] = {0};   // for read EFID
    SimRefreshResponse *param = NULL;

    memset(&simRefreshRspV7, 0, sizeof(RIL_SimRefreshResponse_v7));
    /// M: Vzw SIM application test @{
    char optr[PROPERTY_VALUE_MAX] = {0};
    int lteConnectionStatus[7] = {0};
    property_get("persist.vendor.operator.optr", optr, "");
    /// @}
    LOGD("onSimRefresh: type:%c urc:%s", urc[9], urc);
    switch(urc[9]) { // t point to cmd_deatil tag t[9] mean refresh type
        case '0':
             /// M: Vzw SIM application test @{
            refreshCount++;
            /// @}
            aid_len = decodeStkRefreshAid(urc, &aid);
            simRefreshRspV7.aid = aid;
            simRefreshRspV7.result = SIM_INIT_FULL_FILE_CHANGE;
            /// M: Vzw SIM application test @{
            RIL_requestProxyTimedCallback(resetRefreshCount, NULL, &TIMEVAL_60S,
                getRILChannelId(RIL_STK, rid), "resetRefreshCount");
            /// @}
            break;
        case '1':
            efId_str = decodeStkRefreshFileChange(urc, &cmd , &cmd_length);
            aid_len = decodeStkRefreshAid(urc, &aid);
            simRefreshRspV7.aid = aid;
            simRefreshRspV7.result = SIM_FILE_UPDATE;
            break;
        case '2':
            efId_str = decodeStkRefreshFileChange(urc, &cmd , &cmd_length);
            aid_len = decodeStkRefreshAid(urc, &aid);
            simRefreshRspV7.aid = aid;
            simRefreshRspV7.result = SIM_INIT_FILE_CHANGE;
            break;
        case '3':
            aid_len = decodeStkRefreshAid(urc, &aid);
            simRefreshRspV7.aid = aid;
            simRefreshRspV7.result = SIM_INIT;
            break;
        case '4':
            aid_len = decodeStkRefreshAid(urc, &aid);
            LOGD("onSimRefresh: aid = %s, %d", aid, aid_len);
            param = (SimRefreshResponse*)malloc(sizeof(SimRefreshResponse));
            if (param == NULL) {
                LOGE("SimRefreshResponse malloc fail");
                return;
            }
            memset(param, 0, sizeof(SimRefreshResponse));
            param->simRefreshRsp = (RIL_SimRefreshResponse_v7*)malloc(sizeof(RIL_SimRefreshResponse_v7));
            if (param->simRefreshRsp == NULL) {
                LOGE("SimRefreshResponse->simRefreshRsp malloc fail");
                free(param);
                return;
            }
            memset(param->simRefreshRsp, 0, sizeof(RIL_SimRefreshResponse_v7));
            if (aid != NULL) {
                param->simRefreshRsp->aid = (char*)malloc(strlen(aid) + 1);
                if (param->simRefreshRsp->aid == NULL) {
                    LOGE("SimRefreshResponse->simRefreshRsp->aid malloc fail");
                    free(param->simRefreshRsp);
                    free(param);
                    return;
                }
                strncpy(param->simRefreshRsp->aid, aid, strlen(aid));
                param->simRefreshRsp->aid[strlen(aid)] = '\0';
            }
            param->simRefreshRsp->result = SIM_RESET;
            cpinRetry = 0;
            RIL_requestProxyTimedCallback(queryCpinStatus, param, &TIMEVAL_0S,
                getRILChannelId(RIL_STK, rid), "queryCpinStatus");
            return;
        case '5': // ISIM app_reset
            aid_len = decodeStkRefreshAid(urc, &aid);
            simRefreshRspV7.aid = aid;
            simRefreshRspV7.result = APP_INIT;
            break;
        case '6':
            efId_str = decodeStkRefreshFileChange(urc, &cmd , &cmd_length);
            LOGD("onSimRefresh: efId = %s", efId_str);
            aid_len = decodeStkRefreshAid(urc, &aid);
            LOGD("onSimRefresh: aid = %s, %d", aid, aid_len);

            param = (SimRefreshResponse*) malloc(sizeof(SimRefreshResponse));
            if (param == NULL) {
                LOGE("SimRefreshResponse malloc fail");
                return;
            }
            memset(param, 0, sizeof(SimRefreshResponse));
            param->simRefreshRsp = (RIL_SimRefreshResponse_v7*) malloc(
                    sizeof(RIL_SimRefreshResponse_v7));
            if (param->simRefreshRsp == NULL) {
                LOGE("SimRefreshResponse->simRefreshRsp malloc fail");
                free(param);
                return;
            }
            memset(param->simRefreshRsp, 0, sizeof(RIL_SimRefreshResponse_v7));
            if (aid != NULL) {
                param->simRefreshRsp->aid = (char*) malloc(strlen(aid) + 1);
                if (param->simRefreshRsp->aid == NULL) {
                    LOGE("SimRefreshResponse->simRefreshRsp->aid malloc fail");
                    free(param->simRefreshRsp);
                    free(param);
                    return;
                }
                strncpy(param->simRefreshRsp->aid, aid, strlen(aid));
                param->simRefreshRsp->aid[strlen(aid)] = '\0';
            }
            if (efId_str != NULL) {
                param->efId = (char*) malloc(strlen(efId_str) + 1);
                if (param->efId == NULL) {
                    LOGE("SimRefreshResponse->efId malloc fail");
                    if (param->simRefreshRsp->aid != NULL) {
                        free(param->simRefreshRsp->aid);
                    }
                    free(param->simRefreshRsp);
                    free(param);
                    return;
                }
                strncpy(param->efId, efId_str, strlen(efId_str));
                param->efId[strlen(efId_str)] = '\0';
            }
            param->simRefreshRsp->result = SESSION_RESET;

            cpinRetry = 0;
            RIL_requestProxyTimedCallback(queryCpinStatus, param, &TIMEVAL_0S,
                getRILChannelId(RIL_STK, rid), "queryCpinStatus");
            if (NULL != cmd) {
                free(cmd);
            }
            if (efId_str != NULL) {
                free(efId_str);
                efId_str = NULL;
            }
            return;
        default:
            LOGD("Refresh type does not support.");
            return;
    }
    LOGD("aid = %s, %d.", simRefreshRspV7.aid, aid_len);
    simRefreshRspV7.ef_id = 0;   // default EFID
    if (NULL != efId_str && 4 <= strlen(efId_str)) {
        files_num = strlen(efId_str)/4;
    }
    // Try to read each EFID and send the URC for SIM REFRESH
    if (NULL != efId_str && 0 < strlen(efId_str)) {
        for (i = 0; i < files_num; i++) {
            memcpy(temp_str, efId_str + readIdx, 4);
            temp_str[4] = '\0';
            efId = strtol(temp_str, NULL, 16);
            simRefreshRspV7.ef_id = efId;
            LOGD("onSimRefresh: efId = %X, file numbers = %d", simRefreshRspV7.ef_id, files_num);
            RIL_onUnsolicitedResponse(
                RIL_UNSOL_SIM_REFRESH,
                &simRefreshRspV7, sizeof(RIL_SimRefreshResponse_v7),
                rid);
            readIdx += 4;  // go to next EFID
        }
    } else {
        RIL_onUnsolicitedResponse(
            RIL_UNSOL_SIM_REFRESH,
            &simRefreshRspV7, sizeof(RIL_SimRefreshResponse_v7),
            rid);
    }
    /// M: Vzw SIM application test @{
    LOGD("onSimRefresh: refreshCount = %d", refreshCount);
    if (strcmp("OP12", optr) == 0 && refreshCount == 4) {
        refreshCount = 0;
        lteConnectionStatus[0] = 32;
        RIL_onUnsolicitedResponse(
            RIL_UNSOL_VOLTE_LTE_CONNECTION_STATUS,
            lteConnectionStatus, sizeof(lteConnectionStatus),
            rid);
    }
    /// @}

    if (NULL != cmd) {
        free(cmd);
    }
    if (efId_str != NULL) {
        free(efId_str);
        efId_str = NULL;
    }
}

bool checkAlphaIdExist(char *cmd) {
    int cmdTag = 0;
    int curIdx = 0;

    if (NULL == cmd) {
        return false;
    }
    cmdTag = rild_sms_hexCharToDecInt(cmd, 2);
    LOGD("checkAlphaIdExist cmd %s", cmd);
    //search alpha id tag.
    if (0x05 == cmdTag || 0x85 == cmdTag) {
        if (4 < strlen(cmd)) {
            curIdx += 2;
            cmdTag = rild_sms_hexCharToDecInt(&cmd[curIdx], 2); //length tag
            if (0x00 < cmdTag) {
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }
    return false;
}

char* buildOpenChannelTRStr(char *cmd) {
    int curIdx = 0;
    int cmdLen = 0;
    int cmdTag = 0;
    int openChannelFirstPartLen = 0;
    int bearerDescLen = 0;
    int bufferSizeLen = 0;
    int openChannelTRLen = 0;
    char *pOpenChannelFirstPart = NULL;
    char *pBearerDesc = NULL;
    char *pBufferSize = NULL;
    char* pOpenChannelTR = NULL;

    LOGD("buildOpenChannelTRStr cmd %s", cmd);

    if (NULL != cmd) {
        cmdLen = strlen(cmd);
    } else {
        return NULL;
    }
    // 10: 2( command detail tag) + 2(len) + 2(number of command) +
    //      2(command type) + 2(command qualifier).
    // 8: device tag
    // 6: result tag
    openChannelFirstPartLen = (sizeof(char) * 24) + 1; // 10 + 8 + 6
    pOpenChannelFirstPart = (char*)calloc(1, openChannelFirstPartLen);
    assert(pOpenChannelFirstPart != NULL);
    memset(pOpenChannelFirstPart, 0, openChannelFirstPartLen);
    // 7 = 6 (result tag) + 1 ('\0')
    memcpy(pOpenChannelFirstPart, &(cmd[curIdx]), openChannelFirstPartLen - 7);
    LOGD("pOpenChannelFirstPart cmd %s", pOpenChannelFirstPart);
    *(pOpenChannelFirstPart + 15) = '2'; //8 '2'
    *(pOpenChannelFirstPart + 17) = '1'; //8 '1'
    // append result code: 830122 (6 bytes)
    *(pOpenChannelFirstPart + 18) = '8';
    *(pOpenChannelFirstPart + 19) = '3';
    *(pOpenChannelFirstPart + 20) = '0';
    *(pOpenChannelFirstPart + 21) = '1';
    *(pOpenChannelFirstPart + 22) = '2';
    *(pOpenChannelFirstPart + 23) = '2';
    LOGD("pOpenChannelFirstPart %s", pOpenChannelFirstPart);
    curIdx += 18; // 10+8
    cmdTag = rild_sms_hexCharToDecInt(&cmd[curIdx], 2);
    do {
        //search alpha id tag.
        if (0x05 == cmdTag || 0x85 == cmdTag) {
            if (4 < strlen(&(cmd[curIdx]))) {
                curIdx += 2;
            } else {
                break;
            }
            cmdLen = rild_sms_hexCharToDecInt(&cmd[curIdx], 2);
            curIdx += 2;
            if (0 < cmdLen) {
                curIdx = curIdx + (cmdLen * 2) /*alpha id*/;
            }
        }
        cmdTag = rild_sms_hexCharToDecInt(&cmd[curIdx], 2);
        //search bearer description tag.
        if (0x35 == cmdTag || 0xB5 == cmdTag) {
            if (4 < strlen(&(cmd[curIdx]))) {
                curIdx += 2;
            } else {
                break;
            }
            cmdLen = rild_sms_hexCharToDecInt(&cmd[curIdx], 2);
            curIdx += 2;
            if (0 < cmdLen) {
                curIdx = curIdx - 4;//Back to command tag index.
                bearerDescLen = sizeof(char) * ((cmdLen * 2) + 4);
                pBearerDesc = (char*)calloc(1, bearerDescLen);
                assert(pBearerDesc != NULL);
                memset(pBearerDesc, 0, bearerDescLen);
                memcpy(pBearerDesc, &(cmd[curIdx]), bearerDescLen);
                curIdx = curIdx + bearerDescLen;
                openChannelTRLen += bearerDescLen;
            }
        }
        LOGD("pOpenChannelFirstPart cmd %s", &(cmd[curIdx]));
        cmdTag = rild_sms_hexCharToDecInt(&cmd[curIdx], 2);
        //search buffer size tag.
        if (0x39 == cmdTag || 0xB9 == cmdTag) {
            if (4 < strlen(&(cmd[curIdx]))) {
                curIdx += 2;
            } else {
                break;
            }
            cmdLen = rild_sms_hexCharToDecInt(&cmd[curIdx], 2);
            curIdx += 2;
            if (0 < cmdLen) {
                curIdx = curIdx - 4;//Back to command tag index.
                bufferSizeLen = sizeof(char) * ((cmdLen * 2) + 4);
                pBufferSize = (char*)calloc(1, bufferSizeLen);
                assert(pBufferSize != NULL);
                memset(pBufferSize, 0, bufferSizeLen);
                memcpy(pBufferSize, &(cmd[curIdx]), bufferSizeLen);
                openChannelTRLen += bufferSizeLen;
            }
        }
    } while(false);
    curIdx = 0;
    openChannelTRLen += openChannelFirstPartLen; // 24: for the open channel first part +  1 '\0'
    pOpenChannelTR = (char*)calloc(1, openChannelTRLen);
    assert(pOpenChannelTR != NULL);
    memset(pOpenChannelTR, 0, openChannelTRLen);
    strncat(pOpenChannelTR, pOpenChannelFirstPart, openChannelFirstPartLen - 1);
    LOGD("pOpenChannelTR %s", pOpenChannelTR);
    curIdx += (openChannelFirstPartLen - 1);
    if (NULL != pBearerDesc) {
        memcpy(&(pOpenChannelTR[curIdx]), pBearerDesc, bearerDescLen);
        curIdx += bearerDescLen;
        free(pBearerDesc);
    }
    if (NULL != pBufferSize) {
        memcpy(&(pOpenChannelTR[curIdx]), pBufferSize, bufferSizeLen);
        free(pBufferSize);
    }
    free(pOpenChannelFirstPart);
    LOGD("pOpenChannelTR end %s", pOpenChannelTR);
    return pOpenChannelTR;
}

void onStkProactiveCommand(char* urc, RILChannelCtx* p_channel)
{
    int err = 0, temp_int = 0, type_pos = 0, cmdDetail_pos = 0, alphaId_pos = 0;
    ATResponse *p_response = NULL;
    char *temp_str;
    char *cmd;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    int urc_len = 0;
    bool isStkServiceRunning = false;
    char *pProCmd = NULL;
    char *pTempFullCmd = NULL;
    int cur_cmd = 0;
    char* cachedPtr = NULL;

    if(urc != NULL) urc_len = strlen(urc);

    isStkServiceRunning = getStkFlagByRid(rid, aIs_stk_service_running);
    LOGD("onStkProactiveCommand: isStkServiceRunning:%d", isStkServiceRunning);

    if ( false == isStkServiceRunning) {
        pProCmd = (char*)calloc(1, urc_len + 1);
        if (NULL == pProCmd) {
            LOGE("onStkProactiveCommand: pProCmd is NULL !!!");
            return;
        }
        memset(pProCmd, 0x0, urc_len + 1);
        memcpy(pProCmd, urc, urc_len);
        setStkCachedData(rid, pProactive_cmd, pProCmd);
        setStkFlagByRid(rid, true, aIs_proac_cmd_queued);
        LOGD("STK service is not running yet.[%p]",pProCmd);
        return;
    }

    err = at_tok_start(&urc);

    err = at_tok_nextint(&urc, &temp_int);

    err = at_tok_nextstr(&urc, &temp_str);

    if(temp_str[2] <= '7' ) { /*add comment*/
        type_pos = 10;
        cmdDetail_pos = 4;
        alphaId_pos = 22;
    } else {
        type_pos = 12;
        cmdDetail_pos = 6;
        alphaId_pos = 24;
    }

    switch(checkStkCommandType(&(temp_str[type_pos]))) {
        case CMD_REFRESH:
            onSimRefresh(&(temp_str[type_pos - 6]), p_channel);
            // return;
            break;
        case CMD_DTMF:
            g_stk_at.cmd_type = CMD_DTMF;
            g_stk_at.cmd_res = 0;
            g_stk_at.rid = rid;

            RIL_requestProxyTimedCallback(onStkAtSendFromUrc, NULL, &TIMEVAL_0,
                                          getRILChannelId(RIL_STK, rid), "onStkAtSendFromUrc");

            RIL_onUnsolicitedResponse(
                RIL_UNSOL_STK_EVENT_NOTIFY,
                temp_str, strlen(temp_str),
                rid);
            return;
            break;
        case CMD_OPEN_CHAN:
            if (checkAlphaIdExist(&(temp_str[alphaId_pos]))) {
                //Cache TR of Open channel command.
                setStkFlagByRid(rid, true, aIs_pending_open_channel);
                cachedPtr = buildOpenChannelTRStr(&(temp_str[cmdDetail_pos]));
                pTempFullCmd = getStkCachedDataByRid(rid, pOpenChannelTR);
                if (NULL != pTempFullCmd) {
                    free(pTempFullCmd);
                }
                setStkCachedData(rid, pOpenChannelTR, cachedPtr);

                //Cache Open channel command for redirecting to BipService.
                pProCmd = (char*)calloc(1, strlen(temp_str) + 1);
                assert(pProCmd != NULL);
                memset(pProCmd, 0x0, strlen(temp_str) + 1);
                memcpy(pProCmd, temp_str, strlen(temp_str));
                pTempFullCmd = getStkCachedDataByRid(rid, pOpenChannelCmd);
                if (NULL != pTempFullCmd) {
                    free(pTempFullCmd);
                }
                setStkCachedData(rid, pOpenChannelCmd, pProCmd);
            } else {
                LOGD("Redirect PC to BipService.");
                RIL_onUnsolicitedResponse (
                    RIL_UNSOL_STK_BIP_PROACTIVE_COMMAND,
                    temp_str, strlen(temp_str),
                    rid);
                return;
            }
            break;
        case CMD_CLOSE_CHAN:
        case CMD_RECEIVE_DATA:
        case CMD_SEND_DATA:
        case CMD_GET_CHAN_STATUS:
            RIL_onUnsolicitedResponse (
                RIL_UNSOL_STK_BIP_PROACTIVE_COMMAND,
                temp_str, strlen(temp_str),
                rid);
            return;

        case CMD_SETUP_MENU:
            LOGD("Report SETUP_MENU.");
            freeStkCacheMenuByRid(rid);
            if (isSimInserted(rid)) {
                pProCmd = (char*)calloc(1, strlen(temp_str) + 1);
                if (NULL == pProCmd) {
                    LOGE("onStkProactiveCommand: pProCmd is NULL !!!");
                    break;
                }
                memset(pProCmd, 0x0, strlen(temp_str) + 1);
                memcpy(pProCmd, temp_str, strlen(temp_str));
                setStkCachedData(rid, pStkMenu_cmd, pProCmd);
            }
            break;
        default:
            break;
    }
    RIL_onUnsolicitedResponse(
        RIL_UNSOL_STK_PROACTIVE_COMMAND,
        temp_str, strlen(temp_str),
        rid);

    return;
}

unsigned int findStkCallDuration(char* str)
{
    int length = 0, offset = 0, temp = 0;
    unsigned int duration = 0;
    char temp_str[3] = {0};
    char *end;

    length = strlen(str);

    while(length > 0) {
        if(str[offset] == '8' && str[offset + 1] == '4') {
            memcpy(temp_str, &(str[offset + 6]), 2);
            if(str[offset + 5] == '0') {
                temp = strtoul(temp_str, &end, 16);
                duration = temp * 60000;
            } else if(str[offset + 5] == '1') {
                temp = strtoul(temp_str, &end, 16);
                duration = temp * 1000;
            } else if(str[offset + 5] == '2') {
                temp = strtoul(temp_str, &end, 16);
                duration = temp * 100;
            }
            break;
        } else {
            length -= 2;
            offset += 2;
            memcpy(temp_str, &(str[offset]), 2);
            temp = strtoul(temp_str, &end, 16);
            length -= (2 * temp + 2);
            offset += (2 * temp + 2);
        }
    }
    return duration;
}

void onStkEventNotify(char* urc, RILChannelCtx* p_channel)
{
    int err = 0;
    int temp_int = 0;
    int cmd_type = 0;
    int type_pos = 0;
    int cmd_not_understood = 0; /* mtk02374 20100502*/
    unsigned int duration = 0;
    char *temp_str;
    char *cmd;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    int urc_len = 0;
    bool isStkServiceRunning = false;
    char *pEventNotify = NULL;

    ATResponse *p_response = NULL;

    RIL_SOCKET_ID *pollSimId = &s_pollSimId;
    if (rid == RIL_SOCKET_1) {
        pollSimId = &s_pollSimId;
#if (SIM_COUNT >= 2)
    } else if (rid == RIL_SOCKET_2) {
        pollSimId = &s_pollSimId2;
#endif
#if (SIM_COUNT >= 3) /* Gemini plus 3 SIM*/
    } else if (rid == RIL_SOCKET_3) {
        pollSimId = &s_pollSimId3;
#endif
#if (SIM_COUNT >= 4) /* Gemini plus 4 SIM*/
    } else if (rid == RIL_SOCKET_4) {
        pollSimId = &s_pollSimId4;
#endif
    }

    if(urc != NULL) urc_len = strlen(urc);

    isStkServiceRunning = getStkFlagByRid(rid, aIs_stk_service_running);
    LOGD("onStkEventNotify: isStkServiceRunning:%d", isStkServiceRunning);

    if (false == isStkServiceRunning) {
        if (checkEventNotifyFreeBuffer(rid) > 0) {
            pEventNotify = (char*)calloc(1, urc_len + 1);
            if (NULL == pEventNotify) {
                LOGE("onStkEventNotify: pEventNotify is NULL !!!");
                return;
            }
            memset(pEventNotify, 0x0, urc_len + 1);
            memcpy(pEventNotify, urc, urc_len);
            setStkQueuedEventNotifyWithRid(rid, pEventNotify);
            setStkFlagByRid(rid, true, aIs_event_notify_queued);
            LOGD("STK service is not running yet.[%p]", pEventNotify);
        }
        return;
    }

    err = at_tok_start(&urc);
    err = at_tok_nextint(&urc, &temp_int);
    err = at_tok_nextstr(&urc, &temp_str);
    if(temp_str[2] <= '7' ) { /*add comment*/
        type_pos = 10;
    } else if(temp_str[2] > '7' ) {
        type_pos = 12;
    }
    cmd_not_understood = checkStkCmdDisplay(&(temp_str[type_pos - 6])); /*temp_str[type_pos -6] points to cmd_detail tag*/
    switch(checkStkCommandType(&(temp_str[type_pos]))) {
        case CMD_REFRESH:
            onSimRefresh(&(temp_str[type_pos - 6]), p_channel);
            break;
        case CMD_SETUP_CALL:
            cmd_type = CMD_SETUP_CALL;
            if(cmd_not_understood == 0) {
                duration = findStkCallDuration(&temp_str[type_pos - 6]); /*temp_str[type_pos -6] points to cmd_detail tag*/
            } else {
                g_stk_at.cmd_type = CMD_SETUP_CALL;
                g_stk_at.cmd_res = 50;
                g_stk_at.rid = rid;
                RIL_requestProxyTimedCallback(onStkAtSendFromUrc, NULL, &TIMEVAL_0,
                                              getRILChannelId(RIL_STK, rid), "onStkAtSendFromUrc");

            }
            break;
        case CMD_SEND_SMS:
            g_sms_at[rid].cmd_type = CMD_SEND_SMS;
            g_sms_at[rid].cmd_res = 0;
            g_sms_at[rid].rid = rid;
            RIL_requestProxyTimedCallback(onStkAtSendSmsFromUrc, pollSimId, &TIMEVAL_SMS,
                                          getRILChannelId(RIL_STK, rid), "onStkAtSendFromUrc");

            break;
        case CMD_SEND_SS:
            g_stk_at.cmd_type = CMD_SEND_SS;
            if(cmd_not_understood == 0) {
                g_stk_at.cmd_res = 0;
            } else {
                g_stk_at.cmd_res = 50;
            }

            g_stk_at.rid = rid;
            RIL_requestProxyTimedCallback(onStkAtSendFromUrc, NULL, &TIMEVAL_0,
                                          getRILChannelId(RIL_STK, rid), "onStkAtSendFromUrc");

            break;
        case CMD_SEND_USSD:
            g_stk_at.cmd_type = CMD_SEND_USSD;
            if(cmd_not_understood == 0) {
                g_stk_at.cmd_res = 0;
            } else {
                g_stk_at.cmd_res = 50;
            }
            g_stk_at.rid = rid;
            RIL_requestProxyTimedCallback(onStkAtSendFromUrc, NULL, &TIMEVAL_0,
                                          getRILChannelId(RIL_STK, rid), "onStkAtSendFromUrc");

            break;
        case CMD_DTMF:
            g_stk_at.cmd_type = CMD_DTMF;
            g_stk_at.cmd_res = 0;
            g_stk_at.rid = rid;

            RIL_requestProxyTimedCallback(onStkAtSendFromUrc, NULL, &TIMEVAL_0,
                                          getRILChannelId(RIL_STK, rid), "onStkAtSendFromUrc");

            break;
        default:
            break;
    }

    RIL_onUnsolicitedResponse(
        RIL_UNSOL_STK_EVENT_NOTIFY,
        temp_str, strlen(temp_str),
        rid);
    if(CMD_SETUP_CALL == cmd_type) {
        RIL_onUnsolicitedResponse(
            RIL_UNSOL_STK_CALL_SETUP,
            &duration, sizeof(duration),
            rid);
    }
    return;
}

extern int rilStkMain(int request, void *data, size_t datalen, RIL_Token t)
{
    switch (request) {
        case RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND:
            MTK_REQUEST_STK_SEND_ENVELOPE_COMMAND(data, datalen, t);
            break;
        case RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS:
            MTK_REQUEST_STK_SEND_ENVELOPE_COMMAND_WITH_STATUS(data, datalen, t);
            break;
        case RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE:
            MTK_REQUEST_STK_SEND_TERMINAL_RESPONSE(data, datalen, t);
            break;
        case RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM:
        case RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM_WITH_RESULT_CODE:
            MTK_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM(data, datalen, t);
            break;
        case RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING:
            MTK_REQUEST_REPORT_STK_SERVICE_IS_RUNNING(data, datalen, t);
            break;
        case RIL_REQUEST_SET_STK_UTK_MODE:
            MTK_RIL_REQUEST_SET_STK_UTK_MODE(data, datalen, t);
            break;
        default:
            return 0; /* no matched requests */
            break;
    }

    return 1; /* request found and handled */
}

extern int rilStkUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel)
{
    RIL_STK_UNUSED_PARM(sms_pdu);

    if(strStartsWith(s, "+STKPCI: 0")) {
        MTK_UNSOL_STK_PROACTIVE_COMMAND((char *)s, p_channel);
    } else if(strStartsWith(s, "+STKPCI: 1")) {
        MTK_UNSOL_STK_EVENT_NOTIFY((char *)s, p_channel);
    } else if(strStartsWith(s, "+STKPCI: 2")) {
        MTK_UNSOL_STK_SESSION_END((char *)s, p_channel);
    } else if(strStartsWith(s, "+STKCTRL:")) {
       // In this way, send to APP will show messy code, it is useless, mark it.
       // If need this future, need design how to show a message according requirement
       //  MTK_UNSOL_STK_CALL_CTRL((char *)s, p_channel);
    } else {
        return 0;
    }
    return 1;
}

int requestSwitchStkUtkMode(int mode, RIL_Token t) {
    int err = 0;
    ATResponse *p_response = NULL;
    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    LOGD("requestSwitchStkUtkMode(), mode=%d.", mode);
    // AT+EUTK
    char* cmd;
    err = asprintf(&cmd, "AT+EUTK=%d", mode);
    LOGD("requestSwitchStkUtkMode(), send command %s.", cmd);
    err = at_send_command(cmd, &p_response, p_channel);
    free(cmd);
    at_response_free(p_response);
    p_response = NULL;
    return err;
}

void requestSetStkUtkMode(void * data, size_t datalen, RIL_Token t) {
    RIL_STK_UNUSED_PARM(datalen)

    int err;
    int mode = ((int *) data)[0];

    LOGD("requestSetStkUtkMode(), mode=%d", mode);

    RIL_SOCKET_ID rid = getRILIdByChannelCtx(STK_CHANNEL_CTX);
    if (!isSimInserted( rid)) {
        LOGD("requestSetStkUtkMode(),  sim is not inserted, ignore AT+EUTK");
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        return;
    }
    // switch UTK/STK mode.
    err = requestSwitchStkUtkMode(mode, t);

    LOGD("requestSetStkUtkMode(), err=%d.", err);
    if (err < 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
}

int getSimIdfromSocketId(RIL_SOCKET_ID rid) {
    int simId = 0;

    if (RIL_SOCKET_4 == rid) {
        simId = 3;
    } else if (RIL_SOCKET_3 == rid) {
        simId = 2;
    } else if (RIL_SOCKET_2 == rid) {
        simId = 1;
    } else {
        simId = 0;
    }

    return simId;
}

int getSimIdfromToken(RIL_Token t) {
    int simId = 0;

    if (RIL_CHANNEL_SET4_OFFSET <= RIL_queryMyChannelId(t)) {
        simId = 3;
    } else if (RIL_CHANNEL_SET3_OFFSET <= RIL_queryMyChannelId(t)) {
        simId = 2;
    } else if (RIL_CHANNEL_OFFSET <= RIL_queryMyChannelId(t)) {
        simId = 1;
    } else {
        simId = 0;
    }

    return simId;
}

int checkEventNotifyFreeBuffer(RIL_SOCKET_ID rid) {
    int i = 0;
    int simId = getSimIdfromSocketId(rid);
    int freeBufferNum = 0;

    if (simId >= SIM_COUNT) {
        LOGE("checkEventNotifyFreeBuffer: Invalid simId:%d !!!", simId);
        return 0;
    }

    for (i = 0 ; i < EVENT_NOTIFY_BUFFER_LEN ; i++) {
        if (NULL == pEvent_notify[simId][i]) {
            freeBufferNum++;
        }
    }

    LOGD("checkEventNotifyFreeBuffer: freeBufferNum:%d", freeBufferNum);
    return freeBufferNum;
}

uint8_t toByte(char c) {
    if (c >= '0' && c <= '9') return (c - '0');
    if (c >= 'A' && c <= 'F') return (c - 'A' + 10);
    if (c >= 'a' && c <= 'f') return (c - 'a' + 10);

    LOGD("toByte Error: %c", c);
    return 0;
}

bool findComprehensionTlvTag(char* cmd, int start_pos, uint8_t tagSearch,
        int* pTagStart, int* pTagTotal) {
    int end_pos = strlen(cmd);
    int pos = start_pos;
    uint8_t tag;
    uint32_t tagLength;

    while (pos < end_pos) {
        tag = (uint8_t) ((toByte(cmd[pos]) << 4) | toByte(cmd[pos+1]));
        tagLength = (uint32_t) ((toByte(cmd[pos+2]) << 4) | toByte(cmd[pos+3]));

        if (tag == tagSearch) {
            if (pTagStart != NULL) {
                *pTagStart = pos;
            }
            if (pTagTotal != NULL) {
                *pTagTotal = ((tagLength+2)*2);
            }
            LOGD("Found TAG 0x%x at %d, len=%d", tagSearch, pos, ((tagLength+2)*2));
            // RFX_ASSERT((pos+ ((tagLength+2)*2)) <= end_pos);
            return true;
        }

        pos += ((tagLength+2)*2);
    }

    LOGD("TAG 0x%x not found", tagSearch);
    return false;
}

// for BT_SAP start
char* StkbtSapMsgIdToString(MsgId msgId) {
    switch (msgId) {
        case MsgId_RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS:
            return "BT_SAP_CARD_READER_STATUS";
        default:
            return "BT_SAP_UNKNOWN_MSG_ID";
    }
}

/* Response value for Card Reader status
 * bit 8: Card in reader is powered on or not (powered on, this bit=1)
 * bit 7: Card inserted or not (Card inserted, this bit=1)
 * bit 6: Card reader is ID-1 size or not (our device is not ID-1 size, so this bit =0)
 * bit 5: Card reader is present or not (for our device, this bit=1)
 * bit 4: Card reader is removable or not (for our device, it is not removable, so this bit=0)
 * bit 3-1: Identifier of the Card reader (for our device: ID=0)
 * normal case, card reader status of our device = 0x11010000 = 0xD0
 * default case, card reader status of our device = 0x00010000 = 0x10
 */

void requestBtSapGetCardStatus(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid) {
    RIL_STK_UNUSED_PARM(data);
    RIL_STK_UNUSED_PARM(datalen)

    RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ *req = NULL;
    RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP rsp;
    BtSapStatus status = -1;

    LOGD("[BTSAP] requestBtSapGetCardStatus start, (%d)", rid);

    req = (RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ*)
        malloc(sizeof(RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ));
    assert(req != NULL);
    memset(req, 0, sizeof(RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ));

    status = queryBtSapStatus(rid);
    LOGD("[BTSAP] requestBtSapGetCardStatus status : %d", status);

    //decodeStkBtSapPayload(MsgId_RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS, data, datalen, req);
    memset(&rsp, 0, sizeof(RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP));
    rsp.CardReaderStatus = BT_SAP_CARDREADER_RESPONSE_DEFAULT;
    rsp.response = RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_Response_RIL_E_SUCCESS;
    rsp.has_CardReaderStatus = true;   //always true

    if (isSimInserted(rid)) {
        LOGD("[BTSAP] requestBtSapGetCardStatus, Sim inserted");
        rsp.CardReaderStatus = rsp.CardReaderStatus | BT_SAP_CARDREADER_RESPONSE_SIM_INSERT;
    } else {
        LOGD("[BTSAP] requestBtSapGetCardStatus, But sim not inserted");
        rsp.response = RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_Response_RIL_E_GENERIC_FAILURE;
        sendStkBtSapResponseComplete(t, RIL_E_GENERIC_FAILURE, MsgId_RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS,
                &rsp);
        return;
    }

    if (status == BT_SAP_CONNECTION_SETUP || status == BT_SAP_ONGOING_CONNECTION
        || status == BT_SAP_POWER_ON) {
        rsp.CardReaderStatus = rsp.CardReaderStatus | BT_SAP_CARDREADER_RESPONSE_READER_POWER;
    } else {
        // For BT_SAP_INIT, BT_SAP_DISCONNECT and BT_SAP_POWER_OFF, return generic fail
        rsp.response = RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_Response_RIL_E_GENERIC_FAILURE;
    }

    LOGD("[BTSAP] requestBtSapGetCardStatus, CardReaderStatus result : %x", rsp.CardReaderStatus);
    sendStkBtSapResponseComplete(t, RIL_E_SUCCESS, MsgId_RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS,
        &rsp);
    free(req);

    RLOGD("[BTSAP] requestBtSapGetCardStatus end");
}

void decodeStkBtSapPayload(MsgId msgId, void *src, size_t srclen, void *dst) {
    pb_istream_t stream;
    const pb_field_t *fields = NULL;

    RLOGD("[BTSAP] decodeStkBtSapPayload start (%s)", StkbtSapMsgIdToString(msgId));
    if (dst == NULL || src == NULL) {
        RLOGE("[BTSAP] decodeStkBtSapPayload, dst or src is NULL!!");
        return;
    }

    switch (msgId) {
        case MsgId_RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS:
            fields = RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ_fields;
            break;
        default:
            RLOGE("[BTSAP] decodeStkBtSapPayload, MsgId is mistake!");
            return;
    }

    stream = pb_istream_from_buffer((uint8_t *)src, srclen);
    if (!pb_decode(&stream, fields, dst) ) {
        RLOGE("[BTSAP] decodeStkBtSapPayload, Error decoding protobuf buffer : %s", PB_GET_ERROR(&stream));
    } else {
        RLOGD("[BTSAP] decodeStkBtSapPayload, Success!");
    }
}

void sendStkBtSapResponseComplete(RIL_Token t, RIL_Errno ret, MsgId msgId, void *data) {
    const pb_field_t *fields = NULL;
    size_t encoded_size = 0;
    uint32_t written_size = 0;
    size_t buffer_size = 0;
    pb_ostream_t ostream;
    bool success = false;
    ssize_t written_bytes;
    int i = 0;

    RLOGD("[BTSAP] sendStkBtSapResponseComplete, start (%s)", StkbtSapMsgIdToString(msgId));

    switch (msgId) {
        case MsgId_RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS:
            fields = RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_fields;
            break;
        default:
            RLOGE("[BTSAP] sendStkBtSapResponseComplete, MsgId is mistake!");
            return;
    }

    if ((success = pb_get_encoded_size(&encoded_size, fields, data)) &&
            encoded_size <= INT32_MAX) {
        buffer_size = encoded_size;
        uint8_t buffer[buffer_size];
        ostream = pb_ostream_from_buffer(buffer, buffer_size);
        success = pb_encode(&ostream, fields, data);

        if(success) {
            RLOGD("[BTSAP] sendStkBtSapResponseComplete, Size: %zu  Size as written: 0x%x",
                encoded_size, written_size);
            // Send response
            RIL_SAP_onRequestComplete(t, ret, buffer, buffer_size);
        } else {
            RLOGE("[BTSAP] sendStkBtSapResponseComplete, Encode failed!");
        }
    } else {
        RLOGE("Not sending response type %d: encoded_size: %zu. encoded size result: %d",
        msgId, encoded_size, success);
    }
}

extern int rilStkBtSapMain(int request, void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid) {
    switch (request) {
        case MsgId_RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS:
            requestBtSapGetCardStatus(data, datalen, t, rid);
            break;
        default:
            return 0;
            break;
    }

    return 1;
}
// for BT_SAP end

