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
#include <libmtkrilutils.h>
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
#include <cutils/properties.h>
#include <termios.h>
#include <signal.h>
#include <ctype.h>
#include <time.h>

#include <ril_callbacks.h>

#include "ril_data.h"
#include "ril_nw.h"
#include "ril_nws.h"
#include "ril_sim.h"
#include "ril_wp.h"
#include "hardware/ccci_intf.h"


#include "sysenv_utils.h"

#undef LOG_TAG
#define LOG_TAG "RIL-OEM"

#include <log/log.h>
#include "ratconfig.h"

#define OEM_CHANNEL_CTX getRILChannelCtxFromToken(t)

extern int s_md_off;
extern int s_main_loop;
extern int s_simSwitchVersion;
extern int s_isSimSwitching;
extern int s_modemCapability[RIL_SOCKET_NUM];
extern char s_logicalModemId[SIM_COUNT][MAX_UUID_LENGTH];
extern int m_msim_mode[RIL_SOCKET_NUM];

extern int s_enhanceModemOffVersion;

char* s_imei[RIL_SOCKET_NUM] = {0};
char* s_imeisv[RIL_SOCKET_NUM] = {0};
char* s_basebandVersion[RIL_SOCKET_NUM] = {0};
char* s_projectFlavor[RIL_SOCKET_NUM] = {0};
char* s_calData = 0;
int gcf_test_mode = 0;

int isNumericSet = 0;

int recevSimSwitchReqCount[RC_PHASE_FINISH+1] = {0};
int targetSimRid = 0;

static pthread_mutex_t s_simInitStateMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_simInitStateCond = PTHREAD_COND_INITIALIZER;
static int s_simInitState = 0;

extern bool bDssNoResetSupport;
extern pthread_mutex_t proxyMutex[];

#ifdef MTK_RIL
extern void simSwitchClearProxyList();
#endif
extern int getRilProxysNum();

extern int onNwCheckSetUnsolResponseFilter(void *data, size_t datalen, RIL_Token t);
extern void setUnsolResponseFilterLinkCapacityEstimate(bool enable, RIL_Token t);

extern void grabRadioPowerLock(void);
extern void releaseRadioPowerLock(void);
extern void setRadioState(RIL_RadioState newState, RIL_SOCKET_ID rid);
// STK start
extern void setStkServiceRunningFlag(RIL_SOCKET_ID rid, bool flag);
extern void freeStkCacheMenuByRid(RIL_SOCKET_ID rid);
extern void freeStkQueuedProactivCmdByRid(RIL_SOCKET_ID rid);
// STK end
extern void queryActiveMode(RIL_SOCKET_ID rid, RILChannelCtx *p_channel);
extern void queryBearer(RILSubSystemId subsystem, RIL_SOCKET_ID rid);

extern void setIaCache(int iaSimId, const char* iccid, const char* protocol, const int authtype,
        const char* username, const int canHandleIms, const char* apn,
        const char* roamingProtocol);
extern int getAttachApnSupport();
extern int definePdnCtx(const RIL_Default_Bearer_VA_Config_Struct *pDefaultBearerVaConfig,
        const char *requestedApn, int protocol, int availableCid, int authType,
        const char *username, const char* password, RILChannelCtx *pChannel);
extern int get_protocol_type(const char* protocol);

/* ALPS02255832, [SRLTE] Set GSM radio off before power off it. } */
void requestSetTrm(void * data, size_t datalen, RIL_Token t);

void requestSetModemThermal(void * data, size_t datalen, RIL_Token t);
void requestGetPhoneCapability(void * data, size_t datalen, RIL_Token t);

void requestBasebandVersion(void * data __unused, size_t datalen __unused, RIL_Token t)
{
    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    if (s_basebandVersion[rid] == NULL)
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    else
        RIL_onRequestComplete(t, RIL_E_SUCCESS, s_basebandVersion[rid], sizeof(char *));
}

void requestSetMute(void * data, size_t datalen __unused, RIL_Token t)
{
    char *cmd = NULL;
    ATResponse *p_response = NULL;
    int err;

    asprintf(&cmd, "AT+CMUT=%d", ((int *)data)[0]);
    err = at_send_command(cmd, &p_response, OEM_CHANNEL_CTX);
    free(cmd);
    if (err < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_INTERNAL_ERR, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
}

void requestGetMute(void * data __unused, size_t datalen __unused, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err;
    int response;
    char *line = NULL;

    err = at_send_command_singleline("AT+CMUT?", "+CMUT:", &p_response, OEM_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0) {
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &response);
    if (err < 0) goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(int));

    at_response_free(p_response);
    return;

error:
    response = 0;
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(int));
    at_response_free(p_response);
}

// TODO: requestOemHookRaw
void requestOemHookRaw(void * data, size_t datalen, RIL_Token t)
{
    /* atci start */
    ATResponse * p_response = NULL;
    ATLine* p_cur = NULL;
    char* line;
    int i;
    int strLength = 0;
    int err = -1;
    char* buffer = (char *) alloca(datalen+1);
    if (buffer == NULL) {
        RLOGE("OOM");
        goto error;
    }
    strncpy(buffer, data, datalen);
    buffer[datalen] = '\0';
    int index = needToHidenLog(buffer);
    if (index >= 0) {
        RLOGD("data = %s****, length = %d", getHidenLogPreFix(index), (int) datalen);
    } else {
        RLOGD("data = %s, length = %d", buffer, (int) datalen);
    }
    if (strncmp(buffer, "ATV0", 4) == 0 || strncmp(buffer, "atv0", 4) == 0) {
        //RLOGD("set isNumericSet on");
        isNumericSet = 1;
    } else if (strncmp(buffer, "ATV1", 4) == 0 || strncmp(buffer, "atv1", 4) == 0) {
        //RLOGD("set isNumericSet off");
        isNumericSet = 0;
    }

    if (strStartsWith(buffer, "AT+EFUN") || strStartsWith(buffer, "AT+CFUN")) {
        // avoid conflict with normal power on flow
        //RLOGD("requestOemHookRaw: grabRadioPowerLock");
        grabRadioPowerLock();
    }

    err = at_send_command_raw(buffer, &p_response, OEM_CHANNEL_CTX);

    if (strStartsWith(buffer, "AT+EFUN") || strStartsWith(buffer, "AT+CFUN")) {
        //RLOGD("requestOemHookRaw: releaseRadioPowerLock");
        releaseRadioPowerLock();
    }

    if (err < 0) {
        RLOGE("OEM_HOOK_RAW fail");
        goto error;
    }

    strLength += 2; //for the pre tag of the first string in response.

    for (p_cur = p_response->p_intermediates; p_cur != NULL;
        p_cur = p_cur->p_next) {
        strLength += (strlen(p_cur->line) + 2); //M:To append \r\n
    }
    strLength += (strlen(p_response->finalResponse) + 2);

    int size = strLength * sizeof(char) + 1;
    line = (char *) alloca(size);
    if (line == NULL) {
        RLOGE("OOM");
        goto error;
    }
    memset(line, 0, size);

    strncpy(line, "\r\n", 2);
    for (i = 0, p_cur = p_response->p_intermediates; p_cur != NULL;
            p_cur = p_cur->p_next, i++){
        strncat(line, p_cur->line, strlen(p_cur->line));
        strncat(line, "\r\n", 2);
        //RLOGD("line[%d] = <%s>", i, line);
    }
    strncat(line, p_response->finalResponse, strlen(p_response->finalResponse));
    strncat(line, "\r\n", 2);
    //RLOGD("line = <%s>", line);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, line, size);

     at_response_free(p_response);
     return;

error:
    line = (char *) alloca(10);
    if (line == NULL) {
        RLOGE("OOM");
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        memset(line, 0, 10);
        strncpy(line, "\r\nERROR\r\n", 9);
        //RLOGD("line = <%s>", line);
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, line, strlen(line));
    }
    at_response_free(p_response);
    return;
}

void requestAtCmdWithProxy(void * data, size_t datalen __unused, RIL_Token t)
{
    /* sending AT command start */
    ATResponse * p_response = NULL;
    ATLine* p_cur = NULL;
    const char* buffer = (char*)data;
    char* line = NULL;
    int i;
    int strLength = 0;
    int err = -1;

    if (strncmp(buffer, "ATV0", 4) == 0 || strncmp(buffer, "atv0", 4) == 0) {
        isNumericSet = 1;
    } else if (strncmp(buffer, "ATV1", 4) == 0 || strncmp(buffer, "atv1", 4) == 0) {
        isNumericSet = 0;
    }

    err = at_send_command_raw(buffer, &p_response, OEM_CHANNEL_CTX);

    if (err < 0) {
        RLOGE("AT_CMD_P: AT_COMMAND_WITH_PROXY fail");
        goto error;
    }

    for (p_cur = p_response->p_intermediates; p_cur != NULL;
        p_cur = p_cur->p_next) {
        strLength += (strlen(p_cur->line) + 4);
    }
    strLength += (strlen(p_response->finalResponse)+ 4); //  \r\n + OK + \r\n

    int size = strLength + 1; //+1 for '\0'
    line = (char *) calloc(1, size);
    if (line == NULL) {
        RLOGE("OOM");
        return;
    }
    for (i = 0, p_cur = p_response->p_intermediates; p_cur != NULL;
        p_cur = p_cur->p_next, i++)
    {
        strncat(line, "\r\n", 2);
        strncat(line, p_cur->line, strlen(p_cur->line));
        strncat(line, "\r\n", 2);
    }
    strncat(line, "\r\n", 2);
    strncat(line, p_response->finalResponse, strlen(p_response->finalResponse));

    strncat(line, "\r\n", 2);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strLength);

    at_response_free(p_response);

    if (line) free(line);
    return;

error:
    line = (char *) calloc(1, 10);
    if (line != NULL) {
        strncpy(line, "\r\nERROR\r\n", 9);
    }
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, line, strlen(line));
    at_response_free(p_response);
    if (line) free(line);
    return;
}
void requestOemHookStrings(void * data, size_t datalen, RIL_Token t)
{
    int i;
    const char ** cur;
    ATResponse *    p_response = NULL;
    int             err = -1;
    ATLine*         p_cur = NULL;
    char**          line;
    int             strLength = datalen / sizeof(char *);
    RIL_Errno       ret = RIL_E_GENERIC_FAILURE;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    RLOGD("got OEM_HOOK_STRINGS: %lu", (long)datalen);

    if (strLength != 2) {
        /* Non proietary. Loopback! */

        RIL_onRequestComplete(t, RIL_E_SUCCESS, data, datalen);

        return;
    }

    /* For AT command access */
    cur = (const char **)data;

    if (NULL != cur[1] && strlen(cur[1]) != 0) {
        if ((strncmp(cur[1],"+CIMI",5) == 0) ||(strncmp(cur[1],"+CGSN",5) == 0)) {
            err = at_send_command_numeric(cur[0], &p_response, OEM_CHANNEL_CTX);

        } else {
            err = at_send_command_multiline(cur[0],cur[1], &p_response, OEM_CHANNEL_CTX);
        }

    } else {
        err = at_send_command(cur[0],&p_response,OEM_CHANNEL_CTX);
    }

    if (strncmp(cur[0],"AT+EPIN2",8) == 0) {
        SimPinCount retryCounts;
        RLOGW("OEM_HOOK_STRINGS: PIN operation detect");
        getPINretryCount(&retryCounts, t, rid);
    }

    if (err < 0 || NULL == p_response) {
            RLOGE("OEM_HOOK_STRINGS fail");
            goto error;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            ret = RIL_E_SUCCESS;
            break;
        case CME_INCORRECT_PASSWORD:
            ret = RIL_E_PASSWORD_INCORRECT;
            break;
        case CME_SIM_PIN_REQUIRED:
        case CME_SIM_PUK_REQUIRED:
            ret = RIL_E_PASSWORD_INCORRECT;
            break;
        case CME_SIM_PIN2_REQUIRED:
            ret = RIL_E_SIM_PIN2;
            break;
        case CME_SIM_PUK2_REQUIRED:
            ret = RIL_E_SIM_PUK2;
            break;
        case CME_DIAL_STRING_TOO_LONG:
            ret = RIL_E_DIAL_STRING_TOO_LONG;
            break;
        case CME_TEXT_STRING_TOO_LONG:
            ret = RIL_E_TEXT_STRING_TOO_LONG;
            break;
        case CME_MEMORY_FULL:
            ret = RIL_E_SIM_MEM_FULL;
            break;
        case CME_BT_SAP_UNDEFINED:
            ret = RIL_E_BT_SAP_UNDEFINED;
            break;
        case CME_BT_SAP_NOT_ACCESSIBLE:
            ret = RIL_E_BT_SAP_NOT_ACCESSIBLE;
            break;
        case CME_BT_SAP_CARD_REMOVED:
            ret = RIL_E_BT_SAP_CARD_REMOVED;
            break;
        default:
            ret = RIL_E_GENERIC_FAILURE;
            break;
    }

    if (ret != RIL_E_SUCCESS) {
        goto error;
    }

    if (strncmp(cur[0],"AT+ESSP",7) == 0) {
        RLOGI("%s!", cur[0]);
        if(cur[1] == NULL) {
            updateCFUQueryType(cur[0]);
        }
    }

    /* Count response length */
    strLength = 0;

    for (p_cur = p_response->p_intermediates; p_cur != NULL;
        p_cur = p_cur->p_next)
        strLength++;

    if (strLength == 0) {
        char *pRet = strdup("OK");
        RIL_onRequestComplete(t, RIL_E_SUCCESS, &pRet, sizeof(char *));
        free(pRet);
    } else {

        line = (char **) alloca(strLength * sizeof(char *));
        if (line == NULL) {
            RLOGE("OOM");
            goto error;
        }
        for (i = 0, p_cur = p_response->p_intermediates; p_cur != NULL;
            p_cur = p_cur->p_next, i++)
        {
            line[i] = p_cur->line;
        }

        RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strLength * sizeof(char *));
    }

    at_response_free(p_response);

    return;

error:
    RIL_onRequestComplete(t, ret, NULL, 0);

    at_response_free(p_response);
}

void requestScreenState(void * data, size_t datalen __unused, RIL_Token t)
{
    /************************************
    * Disable the URC: ECSQ,CREG,CGREG,CTZV
    * For the URC +CREG and +CGREG
    * we will buffer the URCs when the screen is off
    * and issues URCs when the screen is on
    * So we can reflect the ture status when screen is on
    *************************************/

    int on_off, err;
    ATResponse *p_response = NULL;

    on_off = ((int*)data)[0];
    /// M: check if the request is sent to main SIM. @{
    bool requestToMainSim = true;
    if (getMainProtocolRid() != getRILIdByChannelCtx(getRILChannelCtxFromToken(t))) {
        requestToMainSim = false;
    }
    /// @}

    if (on_off)
    {
        // screen is on
        /* disable mtk optimized +CREG URC report mode, as standard */
        err = at_send_command("AT+ECREG=0", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+ECREG=0");
        at_response_free(p_response);
        p_response = NULL;

        /* Disable Network registration events of the changes in LAC or CID */
        err = at_send_command("AT+CREG=3", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0){
            at_response_free(p_response);
            p_response = NULL;
            err = at_send_command("AT+CREG=2", &p_response, OEM_CHANNEL_CTX);
            if (err != 0 || p_response->success == 0)
                LOGW("There is something wrong with the exectution of AT+CREG=2");
        }
        at_response_free(p_response);
        p_response = NULL;

        /* disable mtk optimized +CGREG URC report mode, as standard */
        err = at_send_command("AT+ECGREG=0", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+ECGREG=0");
        at_response_free(p_response);
        p_response = NULL;

        err = at_send_command("AT+CGREG=3", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0) {
            at_response_free(p_response);
            p_response = NULL;
            err = at_send_command("AT+CGREG=2", &p_response, OEM_CHANNEL_CTX);
            if (err != 0 || p_response->success == 0)
                LOGW("There is something wrong with the exectution of AT+CGREG=2");
        }
        at_response_free(p_response);
        p_response = NULL;

        /* Enable get ECSQ URC */
        err = at_send_command("AT+ECSQ=1", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+ECSQ=2");
        at_response_free(p_response);
        p_response = NULL;
        updateSignalStrength();

        /* Enable PSBEARER URC */
        err = at_send_command("AT+PSBEARER=1", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+PSBEARER=1");
        at_response_free(p_response);
        p_response = NULL;

        if (isImsSupport()) {
            err = at_send_command("AT+CIREG=2", &p_response, OEM_CHANNEL_CTX);
            if (err != 0 || p_response->success == 0)
                LOGW("There is something wrong with the exectution of AT+CIREG=2");
            at_response_free(p_response);
            p_response = NULL;
        }

        /* disable mtk optimized +CEREG URC report mode, as standard */
        err = at_send_command("AT+ECEREG=0", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+ECEREG=0");
        at_response_free(p_response);
        p_response = NULL;

        err = at_send_command("AT+CEREG=3", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0) {
            at_response_free(p_response);
            p_response = NULL;
            err = at_send_command("AT+CEREG=2", &p_response, OEM_CHANNEL_CTX);
            if (err != 0 || p_response->success == 0)
                LOGW("There is something wrong with the exectution of AT+CEREG=2");
        }
        at_response_free(p_response);
        p_response = NULL;

        /* Enable EREGINFO URC */
        err = at_send_command("AT+EREGINFO=1", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+EREGINFO=1");
        at_response_free(p_response);
        p_response = NULL;
    }
    else
    {
        // screen is off
        /* enable mtk optimized +CREG URC report mode,
           report +CREG when stat or act changed for CREG format 2 and 3 */
        err = at_send_command("AT+ECREG=1", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0) {
            // NOT support ECREG, set short format.
            at_response_free(p_response);
            p_response = NULL;
            /* Disable Network registration events of the changes in LAC or CID */
            err = at_send_command("AT+CREG=1", &p_response, OEM_CHANNEL_CTX);
            if (err != 0 || p_response->success == 0)
                LOGW("There is something wrong with the exectution of AT+CREG=1");
        }
        at_response_free(p_response);
        p_response = NULL;

        /* enable mtk optimized +CGREG URC report mode,
           report +CGREG when stat or act changed, for CGREG format 2 and 3 */
        err = at_send_command("AT+ECGREG=1", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0) {
            at_response_free(p_response);
            p_response = NULL;
            err = at_send_command("AT+CGREG=1", &p_response, OEM_CHANNEL_CTX);
            if (err != 0 || p_response->success == 0)
                LOGW("There is something wrong with the exectution of AT+CGREG=1");
        }
        at_response_free(p_response);
        p_response = NULL;

        /* Disable get ECSQ URC */
        err = at_send_command("AT+ECSQ=0", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+ECSQ=0");
        at_response_free(p_response);
        p_response = NULL;

        /* Disable PSBEARER URC */
        err = at_send_command("AT+PSBEARER=0", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+PSBEARER=0");
        at_response_free(p_response);
        p_response = NULL;

        /* enable mtk optimized +CEREG URC report mode,
           report +CGREG when stat or act changed, for CEREG format 2 and 3 */
        err = at_send_command("AT+ECEREG=1", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0) {
            at_response_free(p_response);
            p_response = NULL;
            err = at_send_command("AT+CEREG=1", &p_response, OEM_CHANNEL_CTX);
            if (err != 0 || p_response->success == 0)
                LOGW("There is something wrong with the exectution of AT+CEREG=1");
        }
        at_response_free(p_response);
        p_response = NULL;

        /* Disable EREGINFO URC */
        err = at_send_command("AT+EREGINFO=0", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+EREGINFO=0");
        at_response_free(p_response);
        p_response = NULL;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

extern int queryMainProtocol(RILSubSystemId subsystem)
{
    ATResponse *p_response = NULL;
    int err;
    int response;
    char *line;

    if (SIM_COUNT == 1) {
        /* Only Gemini modem support AT+ES3G */
        return CAPABILITY_3G_SIM1;
    } else {
        err = at_send_command_singleline("AT+ES3G?", "+ES3G:", &p_response, getChannelCtxbyProxy());

        if (err < 0 || p_response->success == 0) {
            goto error;
        }

        line = p_response->p_intermediates->line;

        err = at_tok_start(&line);
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &response);
        if (err < 0) goto error;

        /* Gemini+ , +ES3G response 1: SIM1 , 2: SIM2 , 4:SIM3 ,8: SIM4. For SIM3 and SIM4 we convert to 3 and 4 */
        if(response == 4){
            response = CAPABILITY_3G_SIM3;
        }else if(response == 8){
            response = CAPABILITY_3G_SIM4;
        }

        at_response_free(p_response);

        return response;

    error:
        at_response_free(p_response);
        return CAPABILITY_3G_SIM1;
    }
}

void initRadioCapabilityResponse(RIL_RadioCapability* rc, RIL_RadioCapability* copyFromRC) {
    memset(rc, 0, sizeof(RIL_RadioCapability));
    rc->version = RIL_RADIO_CAPABILITY_VERSION;
    rc->session = copyFromRC->session;
    rc->phase = copyFromRC->phase;
    rc->rat = copyFromRC->rat;
    strncpy(rc->logicalModemUuid, copyFromRC->logicalModemUuid, MAX_UUID_LENGTH - 1);
    rc->status = copyFromRC->status;
}

static bool simSwitchIsAllSimInitDone() {
    for (int i = 0; i < getSimCount(); i++) {
        if ((s_simInitState & (1 << i)) == 0) {
            return false;
        }
    }
    return true;
}

void simSwitchNotifySIMInitDone(int slot_id) {
    pthread_mutex_lock(&s_simInitStateMutex);
    RLOGI("notifySIMInitDone, slot:%d, state:0x%x", slot_id, s_simInitState);
    s_simInitState |= (1 << slot_id);
    if (simSwitchIsAllSimInitDone()) {
        RLOGI("notifySIMInitDone, all done");
        pthread_cond_signal(&s_simInitStateCond);
    }
    pthread_mutex_unlock(&s_simInitStateMutex);
}

static void simSwitchSetSIMInitState(int val) {
    pthread_mutex_lock(&s_simInitStateMutex);
    s_simInitState = val;
    pthread_mutex_unlock(&s_simInitStateMutex);
}

#define NS_PER_S 1000000000
bool simSwitchWaitForMillisecond(long long msec) {
    struct timeval tv;
    struct timespec ts;
    int ret = 0;

    gettimeofday(&tv, (struct timezone *) NULL);
    ts.tv_sec = tv.tv_sec + (msec / 1000);
    ts.tv_nsec = (tv.tv_usec + (msec % 1000) * 1000L ) * 1000L;
    /* assuming tv.tv_usec < 10^6 */
    if (ts.tv_nsec >= NS_PER_S) {
        ts.tv_sec++;
        ts.tv_nsec -= NS_PER_S;
    }

    ret = pthread_cond_timedwait(&s_simInitStateCond, &s_simInitStateMutex, &ts);
    if (ret == ETIMEDOUT) {
        return false;
    }
    return true;
}

static bool simSwitchWaitSIMInitDone() {
    pthread_mutex_lock(&s_simInitStateMutex);
    if (!simSwitchIsAllSimInitDone()) {
        RLOGI("wait sim init, state=%d", s_simInitState);
        if (!simSwitchWaitForMillisecond(5000)) {
            pthread_mutex_unlock(&s_simInitStateMutex);
            RLOGI("wait sim init timeout");
            property_set("vendor.ril.cdma.report.case", "1");
            property_set("vendor.ril.mux.report.case", "2");
            property_set("vendor.ril.muxreport", "1");
            return false;
        }
    }
    pthread_mutex_unlock(&s_simInitStateMutex);
    RLOGI("sim init done");
    return true;
}

int doSimSwitchWithoutSimReset(RIL_Token t) {
    int i;
    RILChannelCtx *p_channel_cmd;
    int err;
    char *cmd = NULL;
    ATResponse *p_response = NULL;
    int isSwitchSuccess = 1;
    int proxy_id = RIL_queryMyProxyIdByThread();
    int p_channel_cmd_idx = -1;

    RLOGI("doSimSwitchWithoutSimReset START");
    for(i = 0; i < getSimCount(); i++) {
        setRadioState(RADIO_STATE_OFF, RIL_SOCKET_1+i);
    }
    p_channel_cmd = getRILChannelCtxFromToken(t);
    // clear IA Cache for SIM switch
    clearIaCacheAndDefineInvalidIa();
    // clear all pending request in rild, TBD
    at_send_command("AT+EBOOT=1", NULL, p_channel_cmd);
    //For C2K Dynamic sim switch
    if (isCdmaLteDcSupport()) {
        at_send_command("AT+ERMS=1", NULL, p_channel_cmd);
    }
    // lock channels
    resetSignalCounter();
    for (i = 0; i < getRilProxysNum(); i++) {
        RLOGI("lock request channel[%d]", i);
        if (i == proxy_id) {
            RLOGI("skip current request channel");
            continue;
        }
        err = pthread_mutex_lock(&proxyMutex[i]);
    }
    for (i = 0; i < getSupportChannels(); i++) {
        if (p_channel_cmd->id == i) {
            p_channel_cmd_idx = i;
            RLOGI("skip terminate cmd channel");
            continue;
        }
        getChannelLock(i, LOCK_READER);
        closeChannel(i);
    }

    int modem_setting_value = 0;
    int old3GSim = queryMainProtocol(RIL_SIM);
    if (isCdmaLteDcSupport()) {
        if (old3GSim == CAPABILITY_3G_SIM1) {
            modem_setting_value = CAPABILITY_3G_SIM2;
        } else {
            modem_setting_value = CAPABILITY_3G_SIM1;
        }
        RLOGI("doSimSwitchWithoutSimReset for C2K,modem_setting : %d", modem_setting_value);
    } else {
        modem_setting_value = CAPABILITY_3G_SIM1 << (targetSimRid);
    }
    simSwitchSetSIMInitState(0);
    asprintf(&cmd, "AT+ESIMMAP=%d", modem_setting_value);
    err = at_send_command(cmd, &p_response, p_channel_cmd);
    free(cmd);
    if (err < 0 || p_response->success == 0) {
        RLOGI("requestSetRadioCapability AT+ESIMMAP fail");
        isSwitchSuccess = 0;
    } else {
        //update active mode
        queryActiveMode(getMainProtocolRid(), p_channel_cmd);
        if (isCdmaLteDcSupport()) {
            setSimSwitchProp(modem_setting_value);
        } else {
            setSimSwitchProp(targetSimRid + 1);
        }
    }
    at_response_free(p_response);
    // kill cmd channel
    if (p_channel_cmd_idx >= 0) {
        getChannelLock(p_channel_cmd_idx, LOCK_READER);
        closeChannel(p_channel_cmd_idx);
    }

    int retry = 1;
    while (retry) {
        // fix converity issue, avoid deadlock
        // usleep(300 * 1000);
        retry = 0;
        if (getSignalCounter() != getSupportChannels()) {
            retry = 1;
        } else {
            resetSignalCounter();
        }
    }

    // switch channel
    switchMuxPath();
    openChannelFds();
    simSwitchClearProxyList();

    sendRadioCapabilityDoneIfNeeded();

    // unlock urc channels first
    for(i = RIL_URC; i < getSupportChannels(); i += RIL_CHANNEL_OFFSET) {
        releaseChannelLock(i, LOCK_READER);
    }

    if (false == simSwitchWaitSIMInitDone()) {
        isSwitchSuccess = false;
    }

    // unlock channels
    for (i = 0; i < getSupportChannels(); i++) {
        if ((i % RIL_CHANNEL_OFFSET) != RIL_URC) {
            releaseChannelLock(i, LOCK_READER);
        }
    }
    for (i = 0; i < getRilProxysNum(); i++) {
        RLOGI("unlock request channel[%d]", i);
        if (i == proxy_id) {
            continue;
        }
        pthread_mutex_unlock(&proxyMutex[i]);
    }

    RLOGI("doSimSwitchWithoutSimReset END isSwitchSuccess: %d", isSwitchSuccess);
    return isSwitchSuccess;
}

int doSimSwitchWithoutMdReset(RIL_Token t) {
    int i;
    RILChannelCtx *p_channel_cmd;
    int err;
    char *cmd = NULL;
    ATResponse *p_response = NULL;
    int isSwitchSuccess = 1;
    int p_channel_cmd_idx = -1;

    RLOGI("doSimSwitchWithoutMdReset START");
    for(i = 0; i < getSimCount(); i++) {
        setRadioState(RADIO_STATE_UNAVAILABLE, RIL_SOCKET_1+i);
        // STK start
        setStkServiceRunningFlag(RIL_SOCKET_1 + i, false);
        freeStkQueuedProactivCmdByRid(RIL_SOCKET_1 + i);
        freeStkCacheMenuByRid(RIL_SOCKET_1 + i);
        // STK end
    }
    p_channel_cmd = getRILChannelCtxFromToken(t);
    // clear IA Cache for SIM switch
    clearIaCacheAndDefineInvalidIa();
    // clear all pending request in rild, TBD
    at_send_command("AT+EBOOT=1", NULL, p_channel_cmd);
    //For C2K Dynamic sim switch
    if (isCdmaLteDcSupport()) {
        at_send_command("AT+ERMS=1", NULL, p_channel_cmd);
    }
    // lock channels
    //RLOGI("******************************************");
    resetSignalCounter();
    for (i = 0; i < getSupportChannels(); i++) {
        if (p_channel_cmd->id == i) {
            p_channel_cmd_idx = i;
            RLOGI("skip terminate cmd channel");
            continue;
        }
        getChannelLock(i, LOCK_RESTART);
        getChannelLock(i, LOCK_READER);
        //RLOGI("******** requestSetRadioCapability: try to lock channel %d, name %s, ret %d, %x",
        //        p_channel_old->id, p_channel_old->myName, err, &p_channel_old->restartMutex);
        closeChannel(i);
    }

    // for C2K dynamic sim switch
  //  int modem_setting_value = CAPABILITY_3G_SIM1 << (targetSimRid);
    int modem_setting_value = 0;
    int old3GSim = queryMainProtocol(RIL_SIM);
    if (isCdmaLteDcSupport()) {
        if (old3GSim == CAPABILITY_3G_SIM1) {
            modem_setting_value = CAPABILITY_3G_SIM2;
        } else {
            modem_setting_value = CAPABILITY_3G_SIM1;
        }
        RLOGI("doSimSwitchWithoutMdReset for C2K,modem_setting : %d", modem_setting_value);
    } else {
        modem_setting_value = CAPABILITY_3G_SIM1 << (targetSimRid);
    }
    asprintf(&cmd, "AT+ESIMMAP=%d", modem_setting_value);
    err = at_send_command(cmd, &p_response, p_channel_cmd);
    free(cmd);
    if (err < 0 || p_response->success == 0) {
        RLOGI("requestSetRadioCapability AT+ESIMMAP fail");
        isSwitchSuccess = 0;
    } else {
        //update active mode
        queryActiveMode(getMainProtocolRid(), p_channel_cmd);
        if (isCdmaLteDcSupport()) {
            setSimSwitchProp(modem_setting_value);
        } else {
            setSimSwitchProp(targetSimRid + 1);
        }
    }
    at_response_free(p_response);
    // kill cmd channel
    if (p_channel_cmd_idx >= 0) {
        //RLOGI("******** requestSetRadioCapability: try to lock channel %d, name %s, ret %d, %x",
        //        p_channel_cmd->id, p_channel_cmd->myName, err, &p_channel_old->restartMutex);
        getChannelLock(p_channel_cmd_idx, LOCK_RESTART);
        getChannelLock(p_channel_cmd_idx, LOCK_READER);
        closeChannel(p_channel_cmd_idx);
    }

    // from Android O, we cannot use the API pthread_kill to check the thread is killed or not
    int retry = 1;
    while (retry) {
        // fix converity issue, avoid deadlock
        // usleep(300 * 1000);
        retry = 0;
        if (getSignalCounter() != getSupportChannels()) {
            retry = 1;
        } else {
            resetSignalCounter();
        }
    }

    // switch channel
    switchMuxPath();
    openChannelFds();
    // SIM fw need get ICCID again
    for(i = 0; i < SIM_COUNT; i++) {
        // release SIM channel(CMD_3)
        //RLOGI("******** requestSetRadioCapability: try to unlock channel %d, name %s, %x",
        //        p_channel_sim->id, p_channel_sim->myName, &p_channel_sim->restartMutex);
        releaseChannelLock(getRILChannelId(RIL_SIM, RIL_SOCKET_1+i), LOCK_READER);
        releaseChannelLock(getRILChannelId(RIL_SIM, RIL_SOCKET_1+i), LOCK_RESTART);
    }
    simSwitchClearProxyList();

    // unlock channels
    for (i = 0; i < getSupportChannels(); i++) {
        // already release CMD_3 channel
        if (((i % RIL_CHANNEL_OFFSET) != RIL_CMD_3) && ((i % RIL_CHANNEL_OFFSET) != RIL_URC)) {
            //RLOGI("******** requestSetRadioCapability: try to unlock channel %d, name %s, %x",
            //        p_channel_old->id, p_channel_old->myName, &p_channel_old->restartMutex);
            releaseChannelLock(i, LOCK_READER);
            releaseChannelLock(i, LOCK_RESTART);
        }
    }

    //RLOGI("******************************************");
    sendRadioCapabilityDoneIfNeeded();

    for (i = RIL_URC; i < getSupportChannels(); i += RIL_CHANNEL_OFFSET) {
        // release URC channel
        releaseChannelLock(i, LOCK_READER);
        releaseChannelLock(i, LOCK_RESTART);
    }
    for(i = 0; i < getSimCount(); i++) {
        setRadioState(RADIO_STATE_OFF, RIL_SOCKET_1+i);
    }
    RLOGI("doSimSwitchWithoutMdReset END isSwitchSuccess: %d", isSwitchSuccess);
    return isSwitchSuccess;
}

extern void requestGetRadioCapability(void * data __unused, size_t datalen __unused, RIL_Token t)
{
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    RIL_RadioCapability* rc = (RIL_RadioCapability*) calloc(1, sizeof(RIL_RadioCapability));
    assert(rc != NULL);
    rc->version = RIL_RADIO_CAPABILITY_VERSION;
    rc->session = 0;
    rc->phase = RC_PHASE_CONFIGURED;
    rc->rat = RAF_GSM;
    rc->status = RC_STATUS_NONE;
    queryBearer(RIL_SIM, rid);
    rc->rat = s_modemCapability[rid];
    RLOGI("requestGetRadioCapability : %d, %d, %d, %d, %s, %d, rild:%d",
            rc->version, rc->session, rc->phase, rc->rat, rc->logicalModemUuid, rc->status, rid);
    strncpy(rc->logicalModemUuid, s_logicalModemId[rid], MAX_UUID_LENGTH - 1);
    rc->logicalModemUuid[MAX_UUID_LENGTH - 1] = '\0';
    RIL_onRequestComplete(t, RIL_E_SUCCESS, rc, sizeof(RIL_RadioCapability));
}

static bool isSimSwitchWithSimReset() {
    char ct3g[PROPERTY_VALUE_MAX] = {0};
    int i;
    if (!isDualCTCard()) {
        return false;
    }
    for (i = 0; i < getSimCount(); i++) {
        property_get(PROPERTY_RIL_CT3G[i], ct3g, "");
        if (strcmp(ct3g, "1") == 0) {
            return true;
        }
    }
    return false;
}

extern int isVsimEnabled();
extern int isPersistVsim();

extern ApplyRadioCapabilityResult applyRadioCapability(RIL_RadioCapability* rc, RIL_Token t)
{
    char * cmd;
    ATResponse *p_response = NULL;
    int err;
    RIL_SOCKET_ID rid = getRILIdByChannelId(RIL_queryCommandChannelId(t));
    int old3GSim = queryMainProtocol(RIL_SIM);

    RLOGI("applyRadioCapability : %d, %d, %d, %d, %d, %d", rc->rat, old3GSim, rid,
        recevSimSwitchReqCount[rc->phase], targetSimRid, rc->rat & RAF_GPRS);

    recevSimSwitchReqCount[rc->phase]++;

    if ((rc->rat & RAF_GPRS) == RAF_GPRS) {
        targetSimRid = rid;
    }

    if ((!isCdmaLteDcSupport()) && (recevSimSwitchReqCount[rc->phase] != SIM_COUNT)) {
        RLOGI("only handle last request for sim switch");
        return ApplyRC_NONEED;
    }

    // IMS framework
    if (isImsSupport()) {
        RLOGI("Disable IMS capability before SIM switch");
        err = at_send_command("AT+EIMS=0", &p_response, getRILChannelCtxFromToken(t));
        at_response_free(p_response);
        p_response = NULL;
    }

    if (s_simSwitchVersion == 2 && isVsimEnabled() != 1 && isPersistVsim() != 1) {
        s_isSimSwitching = 1;
        triggerMal(s_isSimSwitching);
        if(RatConfig_isC2kSupported() == 0){
            memset(m_msim_mode, 0, sizeof(m_msim_mode));
            err = at_send_command("AT+EFUN=0", &p_response, getRILChannelCtxFromToken(t));
        }
    } else {
        int modem_setting_value = CAPABILITY_3G_SIM1 << (targetSimRid);
        if (isCdmaLteDcSupport()){
            if (old3GSim == CAPABILITY_3G_SIM1){
                modem_setting_value = CAPABILITY_3G_SIM2;
            } else {
                modem_setting_value = CAPABILITY_3G_SIM1;
            }
            RLOGI("applyRadioCapability for C2K,modem_setting : %d", modem_setting_value);
        }
        if (RatConfig_isLteFddSupported() || RatConfig_isLteTddSupported()) {
            asprintf(&cmd, "AT+ES3G=%d, %d", modem_setting_value, NETWORK_MODE_GSM_UMTS_LTE);
        } else {
            asprintf(&cmd, "AT+ES3G=%d, %d", modem_setting_value, NETWORK_MODE_WCDMA_PREF);
        }
        err = at_send_command(cmd, &p_response, getRILChannelCtxFromToken(t));
        free(cmd);
        if (err < 0 || p_response->success == 0) {
            at_response_free(p_response);
            return APPLYRC_FAIL;
        }
    }
    at_response_free(p_response);
    return ApplyRC_SUCCESS;
}

extern void requestSetRadioCapability(void * data, size_t datalen __unused, RIL_Token t)
{
    ApplyRadioCapabilityResult applyRcResult = ApplyRC_NONEED;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    char sRcSessionId[32] = {0};
    RIL_RadioCapability rc;

    memcpy(&rc, data, sizeof(RIL_RadioCapability));
    RLOGI("requestSetRadioCapability : %d, %d, %d, %d, %s, %d, rild:%d",
        rc.version, rc.session, rc.phase, rc.rat, rc.logicalModemUuid, rc.status, rid);

    memset(sRcSessionId, 0, sizeof(sRcSessionId));
    sprintf(sRcSessionId,"%d",rc.session);


    RIL_RadioCapability* responseRc = (RIL_RadioCapability*) malloc(sizeof(RIL_RadioCapability));
    assert(responseRc != NULL);
    initRadioCapabilityResponse(responseRc, &rc);
    strncpy(responseRc->logicalModemUuid, s_logicalModemId[rid], MAX_UUID_LENGTH - 1);
    responseRc->logicalModemUuid[MAX_UUID_LENGTH - 1] = '\0';

    switch (rc.phase) {
        case RC_PHASE_START:
            RLOGI("requestSetRadioCapability RC_PHASE_START");
            //property_set(PROPERTY_SET_RC_SESSION_ID[0], sRcSessionId);
            // keep session id with system property
            // after modem reset, send session id back to framework with urc
            property_set(PROPERTY_SET_RC_SESSION_ID[0], sRcSessionId);
            // init recevSimSwitchReqCount
            recevSimSwitchReqCount[RC_PHASE_APPLY] = 0;
            recevSimSwitchReqCount[RC_PHASE_FINISH] = 0;
            responseRc->status = RC_STATUS_SUCCESS;
            RIL_onRequestComplete(t, RIL_E_SUCCESS, responseRc, sizeof(RIL_RadioCapability));
            break;
        case RC_PHASE_FINISH:
            RLOGI("requestSetRadioCapability RC_PHASE_FINISH");
            // transaction of sim switch is done, reset system property
            char sessionIdStr[PROPERTY_VALUE_MAX] = { 0 };
            int sessionId;
            property_get(PROPERTY_SET_RC_SESSION_ID[0], sessionIdStr, "-1");
            sessionId = atoi(sessionIdStr);
            recevSimSwitchReqCount[rc.phase]++;
            if (recevSimSwitchReqCount[rc.phase] == SIM_COUNT) {
                property_set(PROPERTY_SET_RC_SESSION_ID[0], "-1");
            }
            responseRc->phase = RC_PHASE_CONFIGURED;
            responseRc->status = RC_STATUS_SUCCESS;
            if (sessionId != -1) {
                responseRc->session = sessionId;
            }
            RIL_onRequestComplete(t, RIL_E_SUCCESS, responseRc, sizeof(RIL_RadioCapability));
            break;
        case RC_PHASE_APPLY:
            responseRc->status = RC_STATUS_SUCCESS;
            applyRcResult = applyRadioCapability(responseRc, t);

            // send request back to socket before reset radio,
            // or the response may be lost due to modem reset

            RLOGI("requestSetRadioCapability applyRcResult:%d, s_md_off:%d",applyRcResult, s_md_off);
            switch (applyRcResult) {
                case ApplyRC_NONEED:
                    RIL_onRequestComplete(t, RIL_E_SUCCESS, responseRc, sizeof(RIL_RadioCapability));
                    break;
                case APPLYRC_FAIL:
                    s_isSimSwitching = 0;
                    triggerMal(s_isSimSwitching);
                    if (s_md_off == 1) {
                        RIL_onRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, responseRc, sizeof(RIL_RadioCapability));
                    } else {
                        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, responseRc, sizeof(RIL_RadioCapability));
                    }
                    break;
                case ApplyRC_SUCCESS:
                    break;
            }

            // "isVsimEnabled() == 1" means vsim is instered and in working state
            // "isPersistVsim() == 1" means vism feature is enabled but no vsim inserted
            if (s_simSwitchVersion == 1 || isVsimEnabled() == 1 || isPersistVsim() == 1) {
                // only legacy sim switch needs to reset radio
                if (applyRcResult == ApplyRC_SUCCESS) {
                    RIL_onRequestComplete(t, RIL_E_SUCCESS, responseRc, sizeof(RIL_RadioCapability));

                    RLOGI("requestSetRadioCapability resetRadio");
                    property_set("vendor.gsm.ril.eboot", "1");
                    if (isCdmaLteDcSupport()) {
                        resetRadioForSvlte();
                    } else {
                        resetRadio();
                    }
                }
            }  else {
                // only legacy sim switch needs to reset radio
                if (applyRcResult == ApplyRC_SUCCESS) {
                    int ret;
                    if (bDssNoResetSupport && false == isSimSwitchWithSimReset()) {
                        ret = doSimSwitchWithoutSimReset(t);
                    } else {
                        // SIM switch without MD reset case
                        ret = doSimSwitchWithoutMdReset(t);
                    }
                    s_isSimSwitching = 0;
                    triggerMal(s_isSimSwitching);
                    if (ret == 0) {
                        // fail case
                        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, responseRc, sizeof(RIL_RadioCapability));
                    } else {
                        RIL_onRequestComplete(t, RIL_E_SUCCESS, responseRc, sizeof(RIL_RadioCapability));
                    }
                }
            }
            break;
        default:
            RLOGI("requestSetRadioCapability default");
            responseRc->status = RC_STATUS_FAIL;
            RIL_onRequestComplete(t, RIL_E_INVALID_ARGUMENTS, NULL, 0);
            break;
    }
    free(responseRc);
}

void requestSN(RILChannelCtx *p_channel) {
    ATResponse *p_response = NULL;
    int err;
    char *line, *sv;
    // type 5: Serial Number
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    err = at_send_command_singleline("AT+EGMR=0,5" , "+EGMR:",&p_response, p_channel);
    if (err < 0 || p_response->success == 0) {
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if(err < 0) goto error;

    err = at_tok_nextstr(&line, &sv);
    if(err < 0) goto error;

    property_set(PROPERTY_SERIAL_NUMBER, sv);
    LOGD("[RIL%d] Get serial number: %s", rid+1, sv);

    at_response_free(p_response);
    return;
error:
    at_response_free(p_response);
}

void requestGetImei(void * data __unused, size_t datalen __unused, RIL_Token t)
{
    /*RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);

    if (s_imei[rid] == NULL) {
        ATResponse *p_response = NULL;
        int err;

        err = at_send_command_numeric("AT+CGSN", &p_response, NW_CHANNEL_CTX);

        if (err < 0 || p_response->success == 0) {
            RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        } else {
            err = asprintf(s_imei[rid], "%s", p_response->p_intermediates->line);
            if(err >= 0)
                RIL_onRequestComplete(t, RIL_E_SUCCESS, p_response->p_intermediates->line, sizeof(char *));
        }
        at_response_free(p_response);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, s_imei[rid], sizeof(char *));
    }*/

    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    LOGD("Get imei (%d)", rid);
    if (s_imei[rid] == NULL)
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    else
        RIL_onRequestComplete(t, RIL_E_SUCCESS, s_imei[rid], sizeof(char *));

}

void requestGetImeisv(void * data __unused, size_t datalen __unused, RIL_Token t)
{
    /*ATResponse *p_response = NULL;
    int err;
    char *line, *sv;

    err = at_send_command_singleline("AT+EGMR=0,9", "+EGMR:",&p_response, NW_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0) {
        goto error;
    } else {
        line = p_response->p_intermediates->line;

        err = at_tok_start(&line);
        if(err < 0) goto error;

        err = at_tok_nextstr(&line, &sv);
        if(err < 0) goto error;

        RIL_onRequestComplete(t, RIL_E_SUCCESS,
                              sv, sizeof(char *));
    }
    at_response_free(p_response);
    return;
error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);*/

    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    LOGD("Get imeisv (%d)", rid);
    if (s_imeisv[rid] == NULL)
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    else
        RIL_onRequestComplete(t, RIL_E_SUCCESS, s_imeisv[rid], sizeof(char *));
}

extern void triggerMal(int isCapabilitySwitching) {

    if (isCapabilitySwitching)
    {
        // capability switch start, set MAL stopping flag, then stop MAL
        property_set(PROPERTY_SIM_SWITCH_MAL_STOPPING_FLAG, "1");
        property_set(PROPERTY_SIM_SWITCH_CONTROL_MAL, "0");
    }
    else
    {
        // capability switch done, start MAL
        property_set(PROPERTY_SIM_SWITCH_CONTROL_MAL, "1");
        return;
    }
}

extern void flightModeBoot(RILChannelCtx *p_channel)
{
    ATResponse *p_response = NULL;
    RLOGI("Start flight modem boot up");
    // remove setCardSlot because DSDA is phaseout.

    int err;
    if (SIM_COUNT >= 2) {
        err = at_send_command("AT+EFUN=0", &p_response, p_channel);
    } else {
        err = at_send_command("AT+CFUN=4", &p_response, p_channel);
    }
    if (err != 0 || p_response->success == 0)
        LOGW("Start flight modem boot up failed");
    at_response_free(p_response);
}

extern int getMappingSIMByCurrentMode(RIL_SOCKET_ID rid) {
    return GEMINI_SIM_1+rid;
}

extern void upadteSystemPropertyByCurrentMode(int rid, char* key1, char* key2, char* value) {
    if (rid == RIL_SOCKET_1) {
        RLOGI("Update property SIM1 [%s, %s]", key1, value != NULL ? value : "");
        property_set(key1, value);
    } else {
        RLOGI("Update property SIM2 [%s, %s]", key2, value != NULL ? value : "");
        property_set(key2, value);
    }
}

extern void upadteSystemPropertyByCurrentModeGemini(int rid, char* key1, char* key2, char* key3, char* key4, char* value) {
    int pivot = 1;
    int pivotSim;;

    pivotSim = pivot << rid;
    //RLOGI("Update property SIM%d [%s]", pivotSim, value != NULL ? value : "");
    switch(pivotSim) {
        case 1:
            property_set(key1, value);
            break;
        case 2:
            property_set(key2, value);
            break;
        case 4:
            property_set(key3, value);
            break;
        case 8:
            property_set(key4, value);
            break;
        default:
            RLOGE("Update property SIM%d it is unexpected", pivotSim);
            break;
    }
}

extern void bootupGetIccid(RILChannelCtx *p_channel) {
    int result = 0;
    ATResponse *p_response = NULL;
    int err = at_send_command_singleline("AT+ICCID?", "+ICCID:", &p_response, p_channel);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    if (err >= 0 && p_response != NULL) {
        if (at_get_cme_error(p_response) == CME_SUCCESS) {
            char *line;
            char *iccId;
            line = p_response->p_intermediates->line;
            err = at_tok_start (&line);
            if (err >= 0) {
                err = at_tok_nextstr(&line, &iccId);
                if (err >= 0) {
                    char maskIccid[PROPERTY_VALUE_MAX] = {0};
                    givePrintableIccid(iccId, maskIccid);
                    RLOGD("bootupGetIccid[%d] iccid is %s", rid, maskIccid);
                    // Convert all the lower case characters in iccid to upper case
                    stringToUpper(iccId);
                    property_set(PROPERTY_ICCID_SIM[rid], iccId);
                    result = 1;
                } else
                    RLOGD("bootupGetIccid[%d]: get iccid error 2", rid);
            } else {
                RLOGD("bootupGetIccid[%d]: get iccid error 1", rid);
            }
        } else {
            RLOGD("bootupGetIccid[%d]: Error or no SIM inserted!", rid);
        }
    } else {
        RLOGE("bootupGetIccid[%d] Fail", rid);
    }
    at_response_free(p_response);

    if (!result) {
        RLOGE("bootupGetIccid[%d] fail and write default string", rid);
        char cardType[PROPERTY_VALUE_MAX] = {0};
        char *isRuimCard;
        property_get(PROPERTY_RIL_FULL_UICC_TYPE[rid], cardType, "");
        RLOGD("%s: %s", PROPERTY_RIL_FULL_UICC_TYPE[rid], cardType);
        isRuimCard = strstr(cardType, "RUIM");
        if ((isSvlteSupport() || isSrlteSupport()) && (isRuimCard != NULL)) {
            RLOGD("bootupGetIccid RUIM card, not set iccid.sim1 to N/A");
        } else {
             property_set(PROPERTY_ICCID_SIM[rid], "N/A");
        }
    }
}

extern void bootupGetImei(RILChannelCtx *p_channel) {
    ATResponse *p_response = NULL;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    RLOGD("bootupGetImei[%d]", rid);
    int err = at_send_command_numeric("AT+CGSN", &p_response, p_channel);

    if (err >= 0 && p_response->success != 0) {
        err = asprintf(&s_imei[rid], "%s", p_response->p_intermediates->line);
        if(err < 0)
            RLOGE("bootupGetImei[%d] set fail", rid);
    } else {
        RLOGE("bootupGetImei[%d] Fail", rid);
    }
    at_response_free(p_response);
}

extern void bootupGetImeisv(RILChannelCtx *p_channel) {
    ATResponse *p_response = NULL;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    RLOGD("bootupGetImeisv[%d]", rid);
    int err = at_send_command_singleline("AT+EGMR=0,9", "+EGMR:",&p_response, p_channel);

    if (err >= 0 && p_response->success != 0) {
        char* sv = NULL;
        char* line = p_response->p_intermediates->line;
        err = at_tok_start(&line);
        if(err >= 0) {
            err = at_tok_nextstr(&line, &sv);
            if(err >= 0) {
                err = asprintf(&s_imeisv[rid], "%s", sv);
                if(err < 0)
                    RLOGE("bootupGetImeisv[%d] set fail", rid);
            } else {
                RLOGE("bootupGetImeisv[%d] get token fail", rid);
            }
        } else {
            RLOGE("bootupGetImeisv[%d] AT CMD fail", rid);
        }
    } else {
        RLOGE("bootupGetImeisv[%d] Fail", rid);
    }
    at_response_free(p_response);
}

void bootupGetProjectFlavor(RIL_SOCKET_ID rid, RILChannelCtx* p_channel) {
    RLOGE("bootupGetProjectFlavor[%d]", rid);
    ATResponse *p_response = NULL;
    ATResponse *p_response2 = NULL;
    char* line = NULL;
    char* line2 = NULL;
    char* projectName= NULL;
    char* flavor= NULL;
    int err;

    // Add for MD-team query project/flavor properties : project name(flavor)
    err = at_send_command_singleline("AT+EGMR=0,4", "+EGMR:",&p_response, p_channel);
    if (err >= 0 && p_response->success != 0) {
        line = p_response->p_intermediates->line;
        err = at_tok_start(&line);
        if(err >= 0) {
            err = at_tok_nextstr(&line, &projectName);
            if(err < 0)
                RLOGE("bootupGetProjectName[%d] get token fail", rid);
        } else {
            RLOGE("bootupGetProjectName[%d] AT CMD fail", rid);
        }
    } else {
        RLOGE("bootupGetProjectName[%d] Fail", rid);
    }

    //query project property (flavor)
    err = at_send_command_singleline("AT+EGMR=0,13", "+EGMR:",&p_response2, p_channel);
    if (err >= 0 && p_response2->success != 0) {
        line2 = p_response2->p_intermediates->line;
        err = at_tok_start(&line2);
        if(err >= 0) {
            err = at_tok_nextstr(&line2, &flavor);
            if(err < 0)
                RLOGE("bootupGetFlavor[%d] get token fail", rid);
        } else {
            RLOGE("bootupGetFlavor[%d] AT CMD fail", rid);
        }
    } else {
        RLOGE("bootupGetFlavor[%d] Fail", rid);
    }

    //combine string: projectName(flavor)
    err = asprintf(&s_projectFlavor[rid], "%s(%s)",projectName ,flavor);
    if(err < 0) RLOGE("bootupGetProject[%d] set fail", rid);

    if (getMappingSIMByCurrentMode(rid) == GEMINI_SIM_2){
        err = property_set("vendor.gsm.project.baseband.2",s_projectFlavor[rid]);
        if(err < 0) RLOGE("SystemProperty: PROPERTY_PROJECT_2 set fail");
    }else{
        err = property_set("vendor.gsm.project.baseband" ,s_projectFlavor[rid]);
        if(err < 0) RLOGE("SystemProperty: PROPERTY_PROJECT set fail");
    }

    at_response_free(p_response);
    at_response_free(p_response2);
}

extern void bootupGetBasebandVersion(RILChannelCtx *p_channel) {
    ATResponse *p_response = NULL;
    int err, len;
    char *line, *ver, null;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    ver = &null;
    ver[0] = '\0';
    RLOGE("bootupGetBasebandVersion[%d]", rid);
    //Add for MD-team query project/flavor properties : project name(flavor)
    bootupGetProjectFlavor(rid, p_channel);

    err = at_send_command_multiline("AT+CGMR", "+CGMR:",&p_response, p_channel);

    if (err < 0 || p_response->success == 0)
    {
        goto error;
    }
    else if (p_response->p_intermediates != NULL)
    {
        line = p_response->p_intermediates->line;

        err = at_tok_start(&line);
        if(err < 0) goto error;

        //remove the white space from the end
        len = strlen(line);
        while( len > 0 && isspace(line[len-1]) )
            len --;
        line[len] = '\0';

        //remove the white space from the beginning
        while( (*line) != '\0' &&  isspace(*line) )
            line++;

        ver = line;
    }
    else
    {
        // ALPS00295957 : To handle AT+CGMR without +CGMR prefix response
        at_response_free(p_response);
        p_response = NULL;

        RLOGE("Retry AT+CGMR without expecting +CGMR prefix");

        err = at_send_command_raw("AT+CGMR", &p_response, p_channel);

        if (err < 0) {
            RLOGE("Retry AT+CGMR ,fail");
            goto error;
        }

        if(p_response->p_intermediates != NULL)
        {
            line = p_response->p_intermediates->line;

            RLOGD("retry CGMR response = %s", line);

            //remove the white space from the end
            len = strlen(line);
            while( len > 0 && isspace(line[len-1]) )
                len --;
            line[len] = '\0';

            //remove the white space from the beginning
            while( (*line) != '\0' &&  isspace(*line) )
                line++;

            ver = line;
        }
    }
    asprintf(&s_basebandVersion[rid], "%s", ver);
    at_response_free(p_response);
    return;
error:
    at_response_free(p_response);
}

/* ALPS00582073 START */
extern void bootupGetCalData(RILChannelCtx *p_channel)
{
    ATResponse *p_response = NULL;
    int err;

    err = at_send_command_singleline("AT+ECAL?", "+ECAL:", &p_response, p_channel);

    if (err < 0 || p_response->success == 0){
        RLOGE("bootupGetCalData fail,err=%d", err);
    }
    else{
        err = asprintf(&s_calData, "%s", p_response->p_intermediates->line);
        if(err < 0)
            RLOGE("bootupGetCalData set fail,err=%d", err);
        else
            RLOGD("bootupGetCalData s_calData =%s", s_calData);
    }
    at_response_free(p_response);
}
/* ALPS00582073 END */

extern void requestQueryThermal(void *data, size_t datalen __unused, RIL_Token t){
    ATResponse *p_response = NULL;
    char* cmd = NULL;
    int err = 0;

    LOGD("requestQueryThermal Enter,%s", data);

    if(atoi(data) == -1){
        // Enhancement for thermal: Do not query temperature if all radio is off
        int index;
        for (index = 0; index < getSimCount(); index++) {
            RIL_RadioState radioState = getRadioState(index);
            if (RADIO_STATE_ON == radioState) {
                break;
            }
        }
        if (getSimCount() == index) {
            LOGI("requestQueryThermal: all radio is off, return error");
            RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        } else {
            err = at_send_command_singleline("AT+ETHERMAL", "+ETHERMAL:",
                    &p_response, getRILChannelCtxFromToken(t));
            if (err < 0 || p_response->success == 0) {
                LOGD("requestQueryThermal error");
                RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
            } else {
                int err;
                char *line;
                LOGD("requestQueryThermal success");

                line = p_response->p_intermediates->line;
                err = at_tok_start(&line);
                if (err == 0) {
                    RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strlen(line));
                } else {
                    LOGD("requestQueryThermal token start error");
                    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
                }
            }
        }
    } else {
        err = asprintf(&cmd, "AT+ETHERMAL=%s", data);
        if(cmd != NULL){
            err = at_send_command_singleline(cmd, "+ETHERMAL:", &p_response,
                    getRILChannelCtxFromToken(t));
            free(cmd);
            cmd = NULL;
        }
        if (err < 0) {
            LOGD("requestQueryThermal error");
            RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        } else {
            LOGD("requestQueryThermal success");
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        }
    }

    at_response_free(p_response);

    return;
}

void requestGetActivityInfo(void *data __unused, size_t datalen __unused, RIL_Token t){
    ATResponse *p_response = NULL;
    int err;
    char *line;
    RIL_ActivityStatsInfo activityStatsInfo; // RIL_NUM_TX_POWER_LEVELS 5
    int num_tx_levels = 0;
    int op_code = 0;
    RILChannelCtx* p_channel = NULL;
    int value = 0;

    memset(&activityStatsInfo, 0, sizeof(activityStatsInfo));

    // set sleep/idle mode time
    // TODO: need impl. after MD confirm solution.

    // set Tx/Rx power level
    p_channel = getRILChannelCtxFromToken(t);
    err = at_send_command_singleline("AT+ERFTX=11", "+ERFTX:", &p_response, p_channel);
    if (err < 0 || p_response->success == 0) goto error;

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;
    err = at_tok_nextint(&line, &op_code);
    if (err < 0 || op_code != 11) goto error;
    err = at_tok_nextint(&line, &num_tx_levels);
    if (err < 0) goto error;
    if (num_tx_levels > RIL_NUM_TX_POWER_LEVELS) {
        LOGE("requestGetActivityInfo TX level invalid (%d)", num_tx_levels);
        goto error;
    }
    for (int i = 0; i < num_tx_levels; i++) {
        err = at_tok_nextint(&line, &value);
        if (err < 0) goto error;
        activityStatsInfo.tx_mode_time_ms[i] = (uint32_t)value;
    }
    err = at_tok_nextint(&line, &value);
    if (err < 0) goto error;
    activityStatsInfo.rx_mode_time_ms = (uint32_t)value;
    err = at_tok_nextint(&line, &value);
    if (err < 0) goto error;
    activityStatsInfo.sleep_mode_time_ms = (uint32_t)value;
    err = at_tok_nextint(&line, &value);
    if (err < 0) goto error;
    activityStatsInfo.idle_mode_time_ms = (uint32_t)value;

    LOGD("requestGetActivityInfo Tx/Rx (%d, %d, %d, %d, %d, %d, %d, %d, %d)", num_tx_levels,
            activityStatsInfo.tx_mode_time_ms[0], activityStatsInfo.tx_mode_time_ms[1],
            activityStatsInfo.tx_mode_time_ms[2], activityStatsInfo.tx_mode_time_ms[3],
            activityStatsInfo.tx_mode_time_ms[4], activityStatsInfo.rx_mode_time_ms,
            activityStatsInfo.sleep_mode_time_ms, activityStatsInfo.idle_mode_time_ms);

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &activityStatsInfo, sizeof(activityStatsInfo));

    at_response_free(p_response);
    return;

error:
    LOGE("requestGetActivityInfo error");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

void requestGetHardwareConfig(void *data __unused, size_t datalen __unused, RIL_Token t){
    RIL_HardwareConfig hardwareConfigs[2];
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    LOGD("requestGetHardwareConfig");
    memset(hardwareConfigs[0].uuid, 0, MAX_UUID_LENGTH);
    memset(hardwareConfigs[1].uuid, 0, MAX_UUID_LENGTH);
    // type modem
    hardwareConfigs[0].type = RIL_HARDWARE_CONFIG_MODEM;
    strncpy(hardwareConfigs[0].uuid, s_logicalModemId[rid], (MAX_UUID_LENGTH - 1));
    hardwareConfigs[0].state = RIL_HARDWARE_CONFIG_STATE_ENABLED;
    hardwareConfigs[0].cfg.modem.rilModel = 0; // map to DEV_MODEM_RIL_MODEL_SINGLE
    hardwareConfigs[0].cfg.modem.rat = s_modemCapability[rid];
    hardwareConfigs[0].cfg.modem.maxVoice = 1;
    hardwareConfigs[0].cfg.modem.maxData = 1;
    hardwareConfigs[0].cfg.modem.maxStandby = 2;
    // type sim
    //hardwareConfigs[1].uuid = {0};
    hardwareConfigs[1].type = RIL_HARDWARE_CONFIG_SIM;
    strncpy(hardwareConfigs[1].uuid, s_logicalModemId[rid], (MAX_UUID_LENGTH - 1));
    hardwareConfigs[1].state = RIL_HARDWARE_CONFIG_STATE_ENABLED;
    strncpy(hardwareConfigs[1].cfg.sim.modemUuid, s_logicalModemId[rid], (MAX_UUID_LENGTH - 1));
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &hardwareConfigs, sizeof(RIL_HardwareConfig) * 2);

    return;
}

void requestGetDeviceId(void * data __unused, size_t datalen __unused, RIL_Token t)
{
    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    char * responseStr[4] = {0};
    //LOGD("Get imei (%d)(%s)", rid, s_imei[rid]);
    if (s_imei[rid] == NULL) {
        bootupGetImei(p_channel);
    }
    if (s_imeisv[rid] == NULL) {
        bootupGetImeisv(p_channel);
    }
    if (s_imei[rid] == NULL || s_imeisv[rid] == NULL) {
        RIL_onRequestComplete(t, RIL_E_EMPTY_RECORD, NULL, 0);
    } else {
        asprintf(&responseStr[0], "%s", s_imei[rid]);
        asprintf(&responseStr[1], "%s", s_imeisv[rid]);
        asprintf(&responseStr[2], "");
        asprintf(&responseStr[3], "");
        RIL_onRequestComplete(t, RIL_E_SUCCESS, responseStr, 4*sizeof(char *));
        free(responseStr[0]);
        free(responseStr[1]);
        free(responseStr[2]);
        free(responseStr[3]);
    }
}

void requestSendDeviceState(void *data, size_t datalen __unused, RIL_Token t){
    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    RIL_DeviceStateType stateType = ((RIL_DeviceStateType *)data)[0];
    int result = 0;

    switch(stateType) {
        case RIL_DST_LOW_DATA_EXPECTED:
            result = setFastdormancyState(((int*)data)[1], rid);
            if (result == 1) {
                RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            } else {
                RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
            }
            break;
        default:
        case RIL_DST_CHARGING_STATE:
        case RIL_DST_POWER_SAVE_MODE:
            // do nothing
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            break;
    }
}

void requestSetUnsolResponseFilter(void *data, size_t datalen __unused, RIL_Token t){
    RIL_UnsolicitedResponseFilter filter
            = ((RIL_UnsolicitedResponseFilter *)data)[0];
    onNwCheckSetUnsolResponseFilter(data, datalen, t);
    if ((filter & RIL_UR_DATA_CALL_DORMANCY_CHANGED) == RIL_UR_DATA_CALL_DORMANCY_CHANGED) {
        // enable
    } else {
        // disable
    }
    if ((filter & LINK_CAPACITY_ESTIMATE) == LINK_CAPACITY_ESTIMATE) {
        // enable
        setUnsolResponseFilterLinkCapacityEstimate(true, t);
    } else {
        // disable
        setUnsolResponseFilterLinkCapacityEstimate(false, t);
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

extern int triggerCCCIIoctlEx(int request, int *param)
{
    int ret_ioctl_val = -1;
    int ccci_sys_fd = -1;
    char dev_node[32] = {0};
    int enableMd1 = 0, enableMd2 = 0, enableMd5 = 0;
    char prop_value[PROPERTY_VALUE_MAX] = { 0 };

    property_get("ro.vendor.mtk_md1_support", prop_value, "0");
    enableMd1 = atoi(prop_value);
    property_get("ro.vendor.mtk_md2_support", prop_value, "0");
    enableMd2 = atoi(prop_value);
    property_get("ro.vendor.mtk_md5_support", prop_value, "0");
    enableMd5 = atoi(prop_value);

    if (enableMd1) {
        snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_RILD_IOCTL, MD_SYS1));
        LOGD("MD1/SYS1 IOCTL [%s, %d]", dev_node, request);
    } else if(enableMd2) {
        snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_MUXR_IOCTL, MD_SYS2));
        LOGD("MD2/SYS2 IOCTL [%s, %d]", dev_node, request);
    } else {
        snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_RILD_IOCTL, MD_SYS5));
        LOGD("MD1/SYS5 IOCTL [%s, %d]", dev_node, request);
    }

    ccci_sys_fd = open(dev_node, O_RDWR | O_NONBLOCK);

    if (ccci_sys_fd < 0) {
        RLOGD("Open CCCI ioctl port failed [%d]", ccci_sys_fd);
        return -1;
    }

    ret_ioctl_val = ioctl(ccci_sys_fd, request, param);
    LOGD("CCCI ioctl result: ret_val=%d, request=%d, param=%d", ret_ioctl_val, request, *param);

    close(ccci_sys_fd);
    return ret_ioctl_val;
}

extern int triggerCCCIIoctl(int request)
{
    int param = -1;
    int ret_ioctl_val;

    ret_ioctl_val = triggerCCCIIoctlEx(request, &param);

    return ret_ioctl_val;
}

void sendATBeforeEPOF(const char *command, const char *prefix, int times, int interval,
        RILChannelCtx* p_channel) {
    // Noifty modem to deactivate UFPM
    LOGD("Noifty modem to deactivate UFPM");
    ATResponse *p_response = NULL;
    int err = -1;
    int retry = 0;
    do {
        err = at_send_command_singleline(command, prefix, &p_response, p_channel);
        if (p_response != NULL && p_response->success == 0) {
            if (CME_UNKNOWN == at_get_cme_error(p_response)) {
                LOGD("Not support %s", command);
                // modem does not suooprt this command
                break;
            }
        } else {
            // command success
            LOGD("%s command success", command);
            break;
        }
        if (p_response != NULL) {
            at_response_free(p_response);
            p_response = NULL;
        }
        if (interval != 0) {
            usleep(1000 * interval);
        }
    } while (retry++ < times);
    if (p_response != NULL) {
        at_response_free(p_response);
        p_response = NULL;
    }
}

extern int rilOemMain(int request, void *data, size_t datalen, RIL_Token t)
{
    switch (request)
    {
        case RIL_REQUEST_OEM_HOOK_RAW:
        case RIL_LOCAL_REQUEST_AT_COMMAND_WITH_PROXY:
            requestOemHookRaw(data, datalen, t);
            // echo back data
            //RIL_onRequestComplete(t, RIL_E_SUCCESS, data, datalen);
            break;
        case RIL_REQUEST_OEM_HOOK_STRINGS:
            requestOemHookStrings(data,datalen,t);
            break;
        case RIL_REQUEST_SCREEN_STATE:
            requestScreenState(data, datalen, t);
            break;
        case RIL_REQUEST_SET_MUTE:
            requestSetMute(data,datalen,t);
            break;
        case RIL_REQUEST_GET_MUTE:
            requestGetMute(data, datalen, t);
            break;
        case RIL_REQUEST_GET_RADIO_CAPABILITY:
            requestGetRadioCapability(data, datalen, t);
            break;
        case RIL_REQUEST_SET_RADIO_CAPABILITY:
            requestSetRadioCapability(data, datalen, t);
            break;
        case RIL_LOCAL_REQUEST_QUERY_MODEM_THERMAL:
            requestQueryThermal(data, datalen, t);
            break;
        case RIL_REQUEST_BASEBAND_VERSION:
            requestBasebandVersion(data, datalen, t);
            break;
        case RIL_REQUEST_GET_IMEI:
            requestGetImei(data, datalen, t);
            break;
        case RIL_REQUEST_GET_IMEISV:
            requestGetImeisv(data, datalen, t);
            break;
        case RIL_LOCAL_REQUEST_SET_MODEM_THERMAL:
            requestSetModemThermal(data, datalen, t);
            break;
        case RIL_REQUEST_GET_ACTIVITY_INFO:
            requestGetActivityInfo(data, datalen, t);
            break;
        case RIL_REQUEST_GET_HARDWARE_CONFIG:
            requestGetHardwareConfig(data, datalen, t);
            break;
        case RIL_REQUEST_SEND_DEVICE_STATE:
            requestSendDeviceState(data, datalen, t);
            break;
        case RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER:
            requestSetUnsolResponseFilter(data, datalen, t);
            break;
        case RIL_REQUEST_SET_TRM:
            requestSetTrm(data, datalen, t);
            break;
        case RIL_REQUEST_DEVICE_IDENTITY:
        case RIL_REQUEST_GSM_DEVICE_IDENTITY:
            requestGetDeviceId(data, datalen, t);
            break;
        case RIL_REQUEST_MODIFY_MODEM_TYPE:
            requestModifyModem(data, datalen, t);
            break;
        case RIL_LOCAL_REQUEST_RESUME_WORLD_MODE:
            requestResumeWorldModeChanged(data, datalen, t);
            break;
        case RIL_REQUEST_GET_PHONE_CAPABILITY:
            requestGetPhoneCapability(data, datalen, t);
            break;
        case RIL_LOCAL_REQUEST_STORE_MODEM_TYPE:
            requestStoreModem(data, datalen, t);
            break;
        case RIL_LOCAL_REQUEST_RELOAD_MODEM_TYPE:
            requestReloadModem(data, datalen, t);
            break;
        default:
            return 0;  /* no matched request */
            break;
    }

    return 1; /* request found and handled */
}

extern int rilOemUnsolicited(const char *s, const char *sms_pdu __unused, RILChannelCtx* p_channel)
{
    if (strStartsWith(s, "+EWARNING")) {
        int ret;
        char *line = NULL;
        char *dup = strdup(s);
        if (!dup) {
           return 1;
        }
        line = dup;
        ret = at_tok_start(&line);
        if (ret >= 0) {
           LOGD("[handleOemUnsolicited] get warning message %s", s);
           char warningMessage[255];
           warningMessage[0] = 0;
           strncpy(warningMessage, line, 254);

           char modemVersion[256];
           property_get("gsm.version.baseband", modemVersion, "");
           LOGD("[handleOemUnsolicited] md version = %s, msg = %s", modemVersion, warningMessage);
           ret = handleAee(warningMessage, modemVersion);
        }
        free(dup);
        return 1;
    }
    return 0;
}

void getLogicModemId() {
    int i = 0;
    int mdSys = 1;

    for (i = 0; i < getMdMaxSimCount(); i++) {
        sprintf(s_logicalModemId[i], "modem_sys%d_ps%d", mdSys, (i+1));
    }
    // swap if need
    int majorSimRid = RIL_get3GSIM() - 1;
    char tmpLogicalModemId[MAX_UUID_LENGTH];
    if (majorSimRid != 0) {
        strncpy(tmpLogicalModemId, s_logicalModemId[0], (MAX_UUID_LENGTH - 1));
        strncpy(s_logicalModemId[0], s_logicalModemId[majorSimRid], (MAX_UUID_LENGTH - 1));
        strncpy(s_logicalModemId[majorSimRid], tmpLogicalModemId, (MAX_UUID_LENGTH - 1));
    }
}

void requestSetModemThermal(void * data, size_t datalen __unused, RIL_Token t) {
    int modem_on = ((int*) data)[0];
    LOGD("%s: modem_on = %d", __FUNCTION__, modem_on);
    if (modem_on) {
        requestModemPowerOn(NULL, 0, t);
    } else {
        requestModemPowerOff(NULL, 0, t);
    }
}

void requestGetGcfMode(RIL_SOCKET_ID rid)
{
    ATResponse *p_response = NULL;
    int err;
    char *line;
    int ret;
    char *mode = NULL;

    err = at_send_command_singleline("AT+EPCT?", "+EPCT:", &p_response, getChannelCtxbyProxy());

    if (err < 0 || p_response->success == 0) {
        // assume radio is off
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &ret);
    if (err < 0) goto error;

    asprintf(&mode, "%d", ret);

    property_set(PROPERTY_GSM_GCF_TEST_MODE, mode);

    gcf_test_mode = ret;

    free(mode);

    LOGD("[RIL%d] AT+EPCT return %d", rid+1, ret);
    at_response_free(p_response);
    return;

error:
    LOGE("[RIL%d] AT+EPCT return ERROR", rid+1);
    at_response_free(p_response);
}

void requestSetTrm(void * data, size_t datalen  __unused, RIL_Token t) {
    int mode = ((int *)data)[0];
    ATResponse *p_response = NULL;
    ATResponse *p_response2 = NULL;
    int err;

    LOGD("requestSetTrm(): mode = %d", mode );
    if (mode != 3 && (mode != 101)) {
        LOGD("G requestSetTrm, vendor.ril.cdma.report.case ,1");
        property_set("vendor.ril.cdma.report.case", "1");
    }
    onWorldModePrepareTRM(mode);
    switch (mode) {
        case 1:
            property_set("vendor.ril.mux.report.case", "1");
            property_set("vendor.ril.muxreport", "1");
            s_main_loop = 0;
            break;
        case 2:
            property_set("vendor.ril.mux.report.case", "2");
            property_set("vendor.ril.muxreport", "1");
            s_main_loop = 0;
            break;
        case 3:
            err = at_send_command_singleline("AT+EBTSAP=0", "+EBTSAP:", &p_response, getRILChannelCtxFromToken(t));
            if (err < 0 || NULL == p_response) {
                LOGE("TRM AT+EBTSAP=0 Fail , e= %d", err);
                at_response_free(p_response);
                RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
                return;
            }

            if (at_get_cme_error(p_response) != CME_SUCCESS) {
                LOGE("TRM AT+EBTSAP=0 Fail");
                at_response_free(p_response);
                RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
                return;
            }

            err = at_send_command("AT+EBTSAP=1", &p_response2, getRILChannelCtxFromToken(t));

            if (err < 0 || NULL == p_response2) {
                LOGE("TRM AT+EBTSAP=1 Fail, e= %d", err);
                if (err == AT_ERROR_RADIO_UNAVAILABLE) {
                    at_response_free(p_response);
                    at_response_free(p_response2);
                    RIL_onRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
                    LOGD("requestSetTrm() IVSR fail due to modem not available");
                    return;
                }
                goto error;
            }

            // As a last resort if the connect + disconnect pair are not both work
            if (at_get_cme_error(p_response2) != CME_SUCCESS) {
                LOGE("TRM AT+EBTSAP=1 Fail");
                goto error;
            }

            at_response_free(p_response);
            at_response_free(p_response2);
            break;
        case 5:
            property_set("vendor.ril.mux.report.case", "5");
            property_set("vendor.ril.muxreport", "1");
            s_main_loop = 0;
            break;
        case 6:
            property_set("vendor.ril.mux.report.case", "6");
            property_set("vendor.ril.muxreport", "1");
            s_main_loop = 0;
            break;
        case 19:    // Only for hot-plug
            property_set("vendor.ril.mux.report.case", "2");
            property_set("vendor.ril.muxreport", "1");
            s_main_loop = 0;
            break;
        case 101:    // Trigger GSM RILD NE
            LOGD("Request GSM RILD NE.");
            // Make RILD NE here, %s format with int value caused NE.
            LOGD("requestSetTrm mode = %d", mode);
            break;
    }

    LOGD("requestSetTrm() end [%d]", mode);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    at_response_free(p_response);
    at_response_free(p_response2);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    LOGD("requestSetTrm() error");

#ifdef MTK_RIL_MD2
    property_set("vendor.ril.mux.report.case", "6");
#else
    property_set("vendor.ril.mux.report.case", "2");
#endif
    property_set("vendor.ril.cdma.report.case", "1");
    property_set("vendor.ril.muxreport", "1");
    s_main_loop = 0;
}

void requestGetPhoneCapability(void *data __unused, size_t datalen __unused, RIL_Token t){
    RIL_PhoneCapability phoneCapability;

    memset(&phoneCapability, 0, sizeof(RIL_PhoneCapability));

    phoneCapability.maxActiveData = 1;
    phoneCapability.maxActiveInternetData = 1;
    phoneCapability.isInternetLingeringSupported = 0;
    for (int i = 0; i < SIM_COUNT; i++) {
        phoneCapability.logicalModemList[i].modemId = i;
    }
    LOGD("requestGetPhoneCapability (%d, %d, %d)",
            phoneCapability.maxActiveData,
            phoneCapability.maxActiveInternetData,
            phoneCapability.isInternetLingeringSupported);

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &phoneCapability, sizeof(RIL_PhoneCapability));
}

