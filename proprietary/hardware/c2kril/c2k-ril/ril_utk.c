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

/* //hardware/viatelecom/ril/viatelecom-withuim-ril/viatelecom-withuim-ril.c
**
** Copyright 2009, Viatelecom Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
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
#include <getopt.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <termios.h>
#include "atchannel.h"
#include "ril_callbacks.h"
#include <utils/Log.h>
#include "at_tok.h"
#include "icc.h"
#include "misc.h"

#include <cutils/properties.h>
#include <netdb.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/route.h>
#include "ril_sim.h"

 // for BT_SAP
#include <hardware/ril/librilutils/proto/sap-api.pb.h>
#include "pb_decode.h"
#include "pb_encode.h"

#define STK_CHANNEL_CTX getRILChannelCtxFromToken(t)


#define SETUP_MENU_CMD_DETAIL_CODE "81030125"
#define SETUP_EVENT_LIST_CMD_DETAIL_CODE "81030105"

static void requestUTKProfileDownload(void *data, size_t datalen, RIL_Token t);
static void requestUTKCallConfirmed(void *data, size_t datalen, RIL_Token t);
static void requestUTKSendEnvelopeCommand(void *data, size_t datalen, RIL_Token t);
static void requestUTKSendTerminalResponse(void *data, size_t datalen, RIL_Token t);
static void requestUtkSendEnvelopeCommandWithStatus(void *data, size_t datalen, RIL_Token t);

static int checkStkCommandType(char *cmd_str);

//  Cache proactive command start
static bool aIs_stk_service_running = false;
static bool aIs_proac_cmd_queued = false;
static bool aIs_pending_open_channel = false;
static char* pOpenChannelTR = NULL;
static char* pOpenChannel = NULL;

#define UTK_CACHED_CMD_MAX_NUM 10

static char* pProactive_cmd[UTK_CACHED_CMD_MAX_NUM] = {0};

int queued_proc_cmd_num = 0;

void onHandleProactiveCommand(char* cmd);
void onSimRefresh(char* urc);
void sendStkBtSapResponseComplete(RIL_Token t, RIL_Errno ret, MsgId msgId, void *data);
int hexCharToDecInt(char *hex, int length);
int rild_hexCharToDecInt(char *hex, int length);
void requestSetUtkMode(void *data, size_t datalen, RIL_Token t);

void setCacheOpenChannelTR(char *tr) {
    pOpenChannelTR = tr;
}

void setCacheOpenChannel(char *command) {
    pOpenChannel = command;
}

char* getCacheOpenChannel() {
    return pOpenChannel;
}

char* getCacheOpenChannelTR() {
    return pOpenChannelTR;
}

void setUtkFlag(bool flag, bool source) {
    source = flag;
    LOGD("setUtkFlag[%d].", source);
}

char getUtkFlag(bool source) {
    LOGD("getUtkFlag[%d].", source);
    return source;
}

char* getUtkCachedProCmdData(char** source, int number) {
    return *(source + number);
}

void setUtkCachedProCmdData(char** source, char* pCmd, int number) {
    if (number >= UTK_CACHED_CMD_MAX_NUM) {
        LOGD("setUtkCachedProCmdData fail!");
        return;
    }
    *(source + number) = pCmd;
    LOGD("setUtkCachedProCmdData number:%d .",
        number);
}
void setUtkServiceRunningFlag(bool flag) {
    aIs_stk_service_running = flag;
    LOGD("setUtkServiceRunningFlag value:[%d].", aIs_stk_service_running);
}

void resetUtkStatus() {
    setUtkServiceRunningFlag(false);
    queued_proc_cmd_num = 0;
}
void setUtkProCmdQueuedFlag(bool flag) {
    aIs_proac_cmd_queued = flag;
    LOGD("setUtkProCmdQueuedFlag value:[%d].", aIs_proac_cmd_queued);
}

bool checkAlphaIdExist(char *cmd) {
    int cmdTag = 0;
    int curIdx = 0;

    if (NULL == cmd) {
        return false;
    }
    cmdTag = hexCharToDecInt(cmd, 2);
    // LOGD("checkAlphaIdExist cmd %s", cmd);
    // search alpha id tag.
    if (0x05 == cmdTag || 0x85 == cmdTag) {
        if (4 < strlen(cmd)) {
            curIdx += 2;
            cmdTag = hexCharToDecInt(&cmd[curIdx], 2);   // length tag
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
    openChannelFirstPartLen = (sizeof(char) * 24) + 1;   // 10 + 8 + 6
    pOpenChannelFirstPart = (char*)calloc(1, openChannelFirstPartLen);
    assert(pOpenChannelFirstPart != NULL);
    memset(pOpenChannelFirstPart, 0, openChannelFirstPartLen);
    //  7 = 6 (result tag) + 1 ('\0')
    memcpy(pOpenChannelFirstPart, &(cmd[curIdx]), openChannelFirstPartLen - 7);
    LOGD("pOpenChannelFirstPart cmd %s", pOpenChannelFirstPart);
    *(pOpenChannelFirstPart + 15) = '2';   //  8 '2'
    *(pOpenChannelFirstPart + 17) = '1';   //  8 '1'
    // append result code: 830122 (6 bytes)
    *(pOpenChannelFirstPart + 18) = '8';
    *(pOpenChannelFirstPart + 19) = '3';
    *(pOpenChannelFirstPart + 20) = '0';
    *(pOpenChannelFirstPart + 21) = '1';
    *(pOpenChannelFirstPart + 22) = '2';
    *(pOpenChannelFirstPart + 23) = '2';
    LOGD("pOpenChannelFirstPart %s", pOpenChannelFirstPart);
    curIdx += 18;   // 10+8
    cmdTag = hexCharToDecInt(&cmd[curIdx], 2);
    do {
        //  search alpha id tag.
        if (0x05 == cmdTag || 0x85 == cmdTag) {
            if (4 < strlen(&(cmd[curIdx]))) {
                curIdx += 2;
            } else {
                break;
            }
            cmdLen = hexCharToDecInt(&cmd[curIdx], 2);
            curIdx += 2;
            if (0 < cmdLen) {
                curIdx = curIdx + (cmdLen * 2) /*alpha id*/;
            }
        }
        cmdTag = hexCharToDecInt(&cmd[curIdx], 2);
        //  search bearer description tag.
        if (0x35 == cmdTag || 0xB5 == cmdTag) {
            if (4 < strlen(&(cmd[curIdx]))) {
                curIdx += 2;
            } else {
                break;
            }
            cmdLen = hexCharToDecInt(&cmd[curIdx], 2);
            curIdx += 2;
            if (0 < cmdLen) {
                curIdx = curIdx - 4;  //  Back to command tag index.
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
        cmdTag = hexCharToDecInt(&cmd[curIdx], 2);
        //  search buffer size tag.
        if (0x39 == cmdTag || 0xB9 == cmdTag) {
            if (4 < strlen(&(cmd[curIdx]))) {
                curIdx += 2;
            } else {
                break;
            }
            cmdLen = hexCharToDecInt(&cmd[curIdx], 2);
            curIdx += 2;
            if (0 < cmdLen) {
                curIdx = curIdx - 4;  //  Back to command tag index.
                bufferSizeLen = sizeof(char) * ((cmdLen * 2) + 4);
                pBufferSize = (char*)calloc(1, bufferSizeLen);
                assert(pBufferSize != NULL);
                memset(pBufferSize, 0, bufferSizeLen);
                memcpy(pBufferSize, &(cmd[curIdx]), bufferSizeLen);
                openChannelTRLen += bufferSizeLen;
            }
        }
    }  while (false);
    curIdx = 0;
    openChannelTRLen += openChannelFirstPartLen;   //  24: for the open channel first part +  1 '\0'
    pOpenChannelTR = (char*)calloc(1, openChannelTRLen);
    assert(pOpenChannelTR!= NULL);
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

int hexCharToDecInt(char *hex, int length) {
    int i = 0;
    int value, digit;

    for (i = 0, value = 0; i < length && hex[i] != '\0'; i++)  {
        if (hex[i] >= '0' && hex[i] <= '9') {
            digit = hex[i] - '0';
        } else if (hex[i] >= 'A' && hex[i] <= 'F') {
            digit = hex[i] - 'A' + 10;
        } else if (hex[i] >= 'a' && hex[i] <= 'f') {
            digit = hex[i] - 'a' + 10;
        } else {
            return -1;
        }
        value = value*16 + digit;
    }
    return value;
}
//  Cache proactive command end

int rilUtkMain(int request, void *data, size_t datalen, RIL_Token t) {
    switch (request) {
        case RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING:
            requestUTKProfileDownload(data, datalen, t);
            break;
        case RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM:
        case RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM_WITH_RESULT_CODE:
            requestUTKCallConfirmed(data, datalen, t);
            break;
        case RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND:
            requestUTKSendEnvelopeCommand(data, datalen, t);
            break;
        case RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS:
            requestUtkSendEnvelopeCommandWithStatus(data, datalen, t);
            break;
        case RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE:
            requestUTKSendTerminalResponse(data, datalen, t);
            break;
        case RIL_LOCAL_C2K_REQUEST_SET_UTK_MODE:
            requestSetUtkMode(data, datalen, t);
            break;
        default:
            return 0; /* no matched request */
    }
    return 1;
}

int rilUtkUnsolicited(const char *s, const char *sms_pdu) {
    RIL_UTK_UNUSED_PARM(sms_pdu);
    if (strStartsWith(s, "+CRSM:")) {
        char* proactive_cmd = NULL;

        ParseAtcmdCRSM(s, &proactive_cmd);

        if (!proactive_cmd) {
            RIL_onUnsolicitedResponse(RIL_UNSOL_STK_SESSION_END, NULL, 0);
        } else {
            RIL_onUnsolicitedResponse(RIL_UNSOL_STK_PROACTIVE_COMMAND,
                    proactive_cmd, strlen(proactive_cmd));
            free(proactive_cmd);
        }
        return 1;
    } else if (strStartsWith(s, "+UTKURC:")) {
        char* proactive_command = NULL;
        ParseUtkProcmdStr(s, &proactive_command);

        int str_len = 0;
        if (!proactive_command) {
            RIL_onUnsolicitedResponse(RIL_UNSOL_STK_SESSION_END, NULL, 0);
        } else {
            str_len = strlen(proactive_command);
            char result[5] = {0};
            memcpy(result, proactive_command + (str_len - 4), 4);
            memset(proactive_command + (str_len - 4), 0, 4);
            LOGD("NULL != proactive_command");
            RIL_onUnsolicitedResponse(
                RIL_UNSOL_STK_PROACTIVE_COMMAND,
                proactive_command, strlen(proactive_command));
            free(proactive_command);
        }
        return 1;
    } else if (strStartsWith(s, "+UTKIND:")) {
        char* proactive_cmd = NULL;
        ParseUtkRawData(s, &proactive_cmd);
        char *temp_str = NULL;
        if (!proactive_cmd) {
            RIL_onUnsolicitedResponse(RIL_UNSOL_STK_SESSION_END, NULL, 0);
        } else {
            int str_len = strlen(proactive_cmd);
            temp_str = (char*)calloc(1, str_len + 1);
            if (temp_str == NULL) {
                LOGE("+UTKIND: temp_str is NULL!!!!");
                RIL_onUnsolicitedResponse(RIL_UNSOL_STK_SESSION_END, NULL, 0);
                return 1;
            }
            memset(temp_str, 0x0, str_len + 1);
            memcpy(temp_str, proactive_cmd, str_len);

            bool isStkServiceRunning = false;
            isStkServiceRunning = getUtkFlag(aIs_stk_service_running);
            LOGD("rilUtkUnsolicited check %d.", isStkServiceRunning);

            if (false == isStkServiceRunning) {
                setUtkProCmdQueuedFlag(true);
                setUtkCachedProCmdData(pProactive_cmd, temp_str, queued_proc_cmd_num);
                LOGD("UTK service is not running yet.[%s],number[%d]", temp_str, queued_proc_cmd_num);
                queued_proc_cmd_num++;
                free(proactive_cmd);
                return 1;
            } else {
                onHandleProactiveCommand(temp_str);
                free(proactive_cmd);
            }
        }
        return 1;
    } else if (strStartsWith(s, "+UTKNOTIFY:")) {
        char* proactive_cmd = NULL;
        ParseUtkRawData(s, &proactive_cmd);
        if ( proactive_cmd ) {
            if (strStartsWith(proactive_cmd, "81")) {
                //  ignore
                LOGD("ignore +UTKNOTIFY");
            } else {
                char *temp_str = NULL;
                int str_len = strlen(proactive_cmd);
                temp_str = (char*) calloc(1, str_len + 1);
                if (temp_str == NULL) {
                    LOGE("+UTKNOTIFY: temp_str is NULL!!!!");
                    free(proactive_cmd);
                    return 1;
                }
                memset(temp_str, 0x00, str_len + 1);
                memcpy(temp_str, proactive_cmd, str_len);
                // For CT Lab test, can't show toast after send sms
                char optr[PROPERTY_VALUE_MAX] = {0};
                char seg[PROPERTY_VALUE_MAX] = {0};
                property_get("persist.vendor.operator.optr", optr, "");
                property_get("persist.vendor.operator.seg", seg, "");
                if (strcmp(optr, "OP09") == 0
                        && (strcmp(seg, "SEGC") == 0)) {
                    int type_pos = 0;
                    int alphaId_pos = 0;
                    int cmdDetail_pos = 0;
                    if (temp_str[2] <= '7') {
                        type_pos = 10;
                        alphaId_pos = 22;
                        cmdDetail_pos = 4;
                    } else {
                        type_pos = 12;
                        alphaId_pos = 24;
                        cmdDetail_pos = 6;
                    }
                    switch (checkStkCommandType(&(temp_str[type_pos]))) {
                        case CMD_SEND_SMS:
                            LOGD("No need send SEND SMS notify to AP!");
                            return 1;
                        default:
                            LOGD("Go on handling for other notify!");
                    }
                }

                bool isStkServiceRunning = false;
                isStkServiceRunning = getUtkFlag(aIs_stk_service_running);
                LOGD("rilUtkUnsolicited check %d.", isStkServiceRunning);
                if (!isStkServiceRunning) {
                    setUtkProCmdQueuedFlag(true);
                    setUtkCachedProCmdData(pProactive_cmd, temp_str, queued_proc_cmd_num);
                    LOGD("UTK service is not running yet. Cache number[%d]",
                            queued_proc_cmd_num);
                    queued_proc_cmd_num++;
                } else {
                    RIL_onUnsolicitedResponse(RIL_UNSOL_STK_PROACTIVE_COMMAND,
                            proactive_cmd, strlen(proactive_cmd));
                }
            }

            free(proactive_cmd);
        }
        return 1;
    }
    else if (strStartsWith(s, "+UTKCALL:")) {
        //  not used
        return 1;
    } else if (strStartsWith(s, "+EUTKST:")) {
        // remote SIM switch,need reset utk cache flag.
        if (aIs_stk_service_running == true) {
            resetUtkStatus();
        }
        return 1;
    }
    return 0;
}

void onHandleProactiveCommand(char* cmd) {
    // LOGD("onHandleProactiveCommand: %s.", cmd);
    char* cachedPtr = NULL;
    char *pProCmd = NULL;
    int type_pos = 0;
    int alphaId_pos = 0;
    int cmdDetail_pos = 0;
    if (cmd[2] <= '7') {
        type_pos = 10;
        alphaId_pos = 22;
        cmdDetail_pos = 4;
    } else {
        type_pos = 12;
        alphaId_pos = 24;
        cmdDetail_pos = 6;
    }
    switch (checkStkCommandType(&(cmd[type_pos]))) {
        case CMD_REFRESH:
            LOGD("onHandleProactiveCommand CMD_REFRESH");
            onSimRefresh(&(cmd[type_pos - 6]));
            break;
        case CMD_OPEN_CHAN:
            LOGD("onHandleProactiveCommand CMD_OPEN_CHAN");
            if (checkAlphaIdExist(&(cmd[alphaId_pos]))) {
                //  Cache TR of Open channel command.
                setUtkFlag(true, aIs_pending_open_channel);
                cachedPtr = buildOpenChannelTRStr(&(cmd[cmdDetail_pos]));
                setCacheOpenChannelTR(cachedPtr);
                LOGD("onHandleProactiveCommand getCacheOpenChannelTR:%s", getCacheOpenChannelTR());
                 //  Cache Open channel command for redirecting to BipService.
                pProCmd = (char*)calloc(1, strlen(cmd) + 1);
                assert(pProCmd != NULL);
                memset(pProCmd, 0x0, strlen(cmd) + 1);
                memcpy(pProCmd, cmd, strlen(cmd));

                setCacheOpenChannel(pProCmd);
                LOGD("onHandleProactiveCommand getCacheOpenChannel:%s", getCacheOpenChannel());
            } else {
                LOGD("Redirect PC to BipService.");
                RIL_onUnsolicitedResponse(RIL_UNSOL_STK_BIP_PROACTIVE_COMMAND,  cmd, strlen(cmd));
                return;
            }
            break;
        case CMD_SEND_DATA:
        case CMD_RECEIVE_DATA:
        case CMD_CLOSE_CHAN:
        case CMD_GET_CHAN_STATUS:
            RIL_onUnsolicitedResponse(RIL_UNSOL_STK_BIP_PROACTIVE_COMMAND, cmd, strlen(cmd));
            return;
        case CMD_SEND_SMS:
            LOGD("onHandleProactiveCommand ignore SMS notify!");
            return;
        default:
            LOGD("onHandleProactiveCommand default");
            break;
    }
    RIL_onUnsolicitedResponse(RIL_UNSOL_STK_PROACTIVE_COMMAND, cmd, strlen(cmd));
}
static int checkStkCommandType(char *cmd_str) {
    char temp_str[3] = {0};
    char *end;
    int cmd_type = 0;
    memcpy(temp_str, cmd_str, 2);
    cmd_type = strtoul(temp_str, &end, 16);
    cmd_type = 0x7F & cmd_type;
    return cmd_type;
}

char* decodeStkRefreshFileChange(char *str, int **cmd, int *cmd_length) {
    int str_length = 0, file_num = 0, offset = 20, cmdoffset = 0;
    /*offset 20 including cmd_detail tlv: 10, device id tlv:8, file list tag:2*/
    char temp_str[5] = {0};
    char *end;
    str_length = strlen(str);
    char *efId_str;
    int file_idx = 0;

    // file list length: if length < 7F it will use 2 bytes else it will use 4 bytes
    if (str[offset] <= '7') {
        offset += 2;
    } else {
        offset += 4;
    }

    // copy number of files in file list to temp_str
    memcpy(temp_str, str + offset, 2);
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
    LOGD("decodeStkRefreshFileChange file_num = %d, offset = %d", file_num, offset);

    while (offset < str_length && file_num > file_idx) {
        if (((str[offset] == '6') || (str[offset] == '2') || (str[offset] == '4'))
           && ((str[offset + 1] == 'F') || (str[offset+1] == 'f'))) {
            memcpy(temp_str, str + offset, 4);   // copy EFID to temo_str
            strncat(efId_str, temp_str, 4);
            *(*cmd + cmdoffset) = strtoul(temp_str, &end, 16);

            cmdoffset++;
            file_idx++;
        }
        offset += 4;
    }
    return efId_str;
}

int decodeStkRefreshAid(char *urc, char **paid) {
    int offset = 18;  // cmd_details & device identifies
    int refresh_length = strlen(urc) / 2;
    int temp;  // For reading data

    if (offset >= refresh_length) {
        *paid = NULL;
        return 0;
    }

    temp = rild_hexCharToDecInt(&urc[offset], 2);
    offset += 2;
    if (temp == 0x12 || temp == 0x92) {  // file list tag
        temp = rild_hexCharToDecInt(&urc[offset], 2);  // Length of bytes following (bytes)
        if (temp < 0x7F) {
            offset += (2 + temp * 2);
        } else {
            offset += 2;
            temp = rild_hexCharToDecInt(&urc[offset], 2);
            offset += (2 + temp * 2);
        }
        temp = rild_hexCharToDecInt(&urc[offset], 2);  // Number of files
        offset += 2;
    }

    LOGD("decodeStkRefreshAid temp = %02x, offset = %d", temp, offset);

    if (temp == 0x2F || temp == 0xAF) {  // aid tag
        temp = rild_hexCharToDecInt(&urc[offset], 2);  // read the length of AID
        offset += 2;
        *paid = &urc[offset];
        return temp * 2;
    }

    return -1;
}

int rild_hexCharToDecInt(char *hex, int length) {
    int i = 0;
    int value, digit;

    for (i = 0, value = 0; i < length && hex[i] != '\0'; i++) {
        if (hex[i] >= '0' && hex[i] <= '9') {
            digit = hex[i] - '0';
        }
        else if (hex[i] >= 'A' && hex[i] <= 'F') {
            digit = hex[i] - 'A' + 10;
        } else if (hex[i] >='a' && hex[i] <= 'f') {
            digit = hex[i] - 'a' + 10;
        } else {
            return -1;
        }
        value = value*16 + digit;
    }

    return value;
}

void onSimRefresh(char* urc) {
    int *cmd = NULL;
    int cmd_length = 0;
    cmd_length = 2 * sizeof(int);
    cmd = (int *)calloc(1, cmd_length);
    assert(cmd != NULL);

    RIL_SimRefreshResponse_v7 simRefreshRspV7;
    memset(&simRefreshRspV7, 0, sizeof(RIL_SimRefreshResponse_v7));
    int aid_len = 0;
    char *aid = NULL;
    int i = 0;
    int files_num = 0;
    int readIdx = 0;
    int efId = 0;
    char *efId_str = NULL;
    char temp_str[5] = {0};  // for read EFID
    // int volteConnectionStatus[7] = {0};
    LOGD("onSimRefresh: type:%c urc:%s", urc[9], urc);
    switch (urc[9]) {  // urc point to cmd_deatil tag urc[9] mean refresh type
        case '0':
            simRefreshRspV7.result = SIM_INIT_FULL_FILE_CHANGE;
            /*
            volteConnectionStatus[0] = 32;
            RIL_onUnsolicitedResponse(
                RIL_UNSOL_VOLTE_LTE_CONNECTION_STATUS,
                volteConnectionStatus, sizeof(volteConnectionStatus));
                **/
            break;
        case '1':
            simRefreshRspV7.result = SIM_FILE_UPDATE;
            efId_str = decodeStkRefreshFileChange(urc, &cmd , &cmd_length);
            break;
        case '2':
            simRefreshRspV7.result = SIM_INIT_FILE_CHANGE;
            efId_str = decodeStkRefreshFileChange(urc, &cmd , &cmd_length);
            break;
        case '3':
            simRefreshRspV7.result = SIM_INIT;
            break;
        case '4':
            simRefreshRspV7.result = SIM_RESET;
            break;
        case '5':
            simRefreshRspV7.result = APP_INIT;
            break;
        case '6':
            simRefreshRspV7.result = SESSION_RESET;
            efId_str = decodeStkRefreshFileChange(urc, &cmd , &cmd_length);
            break;
        default:
            LOGD("Refresh type does not support.");
            free(cmd);
            return;
    }
    aid_len = decodeStkRefreshAid(urc, &aid);
    simRefreshRspV7.aid = aid;
    LOGD("onSimRefresh: all efId = %s; aid = %s; aid length = %d", efId_str,
        aid, aid_len);

    simRefreshRspV7.ef_id = 0;  // default EFID
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
            LOGD("onSimRefresh: efId = %x, file numbers = %d", simRefreshRspV7.ef_id, files_num);
            RIL_onUnsolicitedResponse(
                RIL_UNSOL_SIM_REFRESH,
                &simRefreshRspV7, sizeof(RIL_SimRefreshResponse_v7));
            readIdx += 4;  // go to next EFID
        }
    } else {
        RIL_onUnsolicitedResponse(
            RIL_UNSOL_SIM_REFRESH,
            &simRefreshRspV7, sizeof(RIL_SimRefreshResponse_v7));
    }

    free(cmd);
}

static void requestUTKProfileDownload(void *data, size_t datalen, RIL_Token t) {
    RIL_UTK_UNUSED_PARM(data);
    RIL_UTK_UNUSED_PARM(datalen);
    setUtkServiceRunningFlag(true);
    int err = at_send_command("AT+UTKPD", NULL, STK_CHANNEL_CTX);
    LOGD("requestUTKProfileDownload err=%d", err);

    LOGD("STK service is running is_proac_cmd_queued[%d].",
    getUtkFlag(aIs_proac_cmd_queued));
    if (true == getUtkFlag(aIs_proac_cmd_queued)) {
        int i = 0;
        for (i = 0; i < UTK_CACHED_CMD_MAX_NUM; i++) {
            char *cmd = (char *)getUtkCachedProCmdData(pProactive_cmd, i);
            // LOGD("Queued Proactive Cmd:[%s].", cmd);
            if (NULL != cmd) {
                char *temp_str1 = NULL;
                int str_len = strlen(cmd);
                temp_str1 = (char*)calloc(1, str_len + 1);
                assert(temp_str1 != NULL);
                memset(temp_str1, 0x0, str_len + 1);
                memcpy(temp_str1, cmd, str_len);
                onHandleProactiveCommand(temp_str1);
                LOGD("clear queud command");
                setUtkCachedProCmdData(pProactive_cmd, NULL, i);
            }
        }
        setUtkProCmdQueuedFlag(false);
    }

    if (err < 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
}

static void requestUTKCallConfirmed(void *data, size_t datalen, RIL_Token t) {
    RIL_UTK_UNUSED_PARM(datalen);
    char *cmd = NULL;
    int err = 0;
    LOGD("requestStkHandleCallSetupRequestedFromSim");
    if (true == getUtkFlag(aIs_pending_open_channel)) {
        setUtkFlag(false, aIs_pending_open_channel);
        if (((int *)data)[0] == 1) {
            // redirect OPEN CHANNEL to BipService.
            cmd = getCacheOpenChannel();
            if (NULL == cmd) {
                LOGD("cmd is null.");
                return;
            }
            //LOGD("cmd: %s.", cmd);
            RIL_onUnsolicitedResponse(
                RIL_UNSOL_STK_BIP_PROACTIVE_COMMAND,
                cmd, strlen(cmd));
            return;
        } else {
            //  response TR of OPEN CAHNNEL to SIM directlly.
            char* cachedTR = getCacheOpenChannelTR();
            LOGD("cachedTR: %s.", cachedTR);
            requestUTKSendTerminalResponse(cachedTR, strlen(cachedTR), t);
            return;
        }
    }
    asprintf(&cmd, "AT+UTKCONF=%d", ((int *)data)[0]);
    err = at_send_command(cmd, NULL, STK_CHANNEL_CTX);
    free(cmd);

    if (err < 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
}

static void requestUTKSendEnvelopeCommand(void *data, size_t datalen, RIL_Token t) {
    RIL_UTK_UNUSED_PARM(datalen);
    int err;
    char *cmd = NULL;
    char* cmd_data = (char*)data;

    //  asprintf(&cmd, "at+crsm=194,0,0,0,0,%d,\"%s\"", strlen(cmd_data) / 2, cmd_data);
    asprintf(&cmd, "AT+UTKENV=%s", cmd_data);
    err = at_send_command(cmd, NULL, STK_CHANNEL_CTX);
    free(cmd);

    if (err == 0) {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }
}

static void requestUtkSendEnvelopeCommandWithStatus(void *data, size_t datalen, RIL_Token t) {
     RIL_UTK_UNUSED_PARM(datalen);
    ATResponse *p_response = NULL;
    int err;
    char* cmd = NULL;
    char *line = NULL;
    RIL_SIM_IO_Response sr;
    char* hexStr = (char*) data;
    int data_len = 0;

    memset(&sr, 0, sizeof(sr));

    if (hexStr != NULL) {
        data_len = strlen(hexStr);
    }
    asprintf(&cmd, "AT+CRSM=194,0,0,0,0,%d,\"%s\"", data_len, (char *)data);

    err = at_send_command_singleline(cmd, "+CRSM:", &p_response, STK_CHANNEL_CTX);
    if (err < 0 || p_response == NULL || (p_response != NULL && p_response->success == 0) ||
            (p_response != NULL && p_response->p_intermediates == NULL)) {
        goto error;
    }

    line = p_response->p_intermediates->line;
    if (NULL == line) {
        LOGE("requestUtkSendEnvelopeCommandWithStatus ok but no intermediates.");
        goto error;
    }

    LOGD("CRSM, [%s]", line);
    err = at_tok_start(&line);
    if (err < 0) {
        LOGE("get +CRSM: error.");
        goto error;
    }
    err = at_tok_nextint(&line, &(sr.sw1));
    if (err < 0) goto error;
    err = at_tok_nextint(&line, &(sr.sw2));
    if (err < 0) goto error;
    err = at_tok_nextstr(&line, &(sr.simResponse));
    if (err < 0) {
        LOGE("response data is null.");
        memset(&(sr.simResponse), 0x00, sizeof(sr.simResponse));
    }
    LOGD("requestUtkSendEnvelopeCommandWithStatus: %02x, %02x", sr.sw1, sr.sw2);

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &sr, sizeof(sr));
    free(cmd);
    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    free(cmd);
    at_response_free(p_response);
    return;
}

static void requestUTKSendTerminalResponse(void *data, size_t datalen, RIL_Token t) {
    RIL_UTK_UNUSED_PARM(datalen);
    int err;
    char *cmd = NULL;
    char* cmd_data = (char*)data;
    //  The TR of SETUP_EVENT_LIST and SETUP_MENU will return at MD.
    if (strStartsWith((char*)data, SETUP_EVENT_LIST_CMD_DETAIL_CODE) ||
           strStartsWith((char*)data, SETUP_MENU_CMD_DETAIL_CODE)) {
        LOGD("Ignore TR of set up event list and set up menu.");
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        return;
    }
    //  asprintf(&cmd, "at+crsm=20,0,0,0,0,%d,\"%s\"", strlen(cmd_data) / 2, cmd_data);
    asprintf(&cmd, "AT+UTKTERM=%s", cmd_data);
    err = at_send_command(cmd, NULL, STK_CHANNEL_CTX);
    free(cmd);

    if ( err == 0 )
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    else
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

/// M: [C2K][IR] Support CT 3g dual Mode card IR feature. @{
/* Switch UTK/STK mode. */
int switchStkUtkMode(int mode, RIL_Token t) {
    int err = 0;
    ATResponse *p_response = NULL;
    LOGD("ril_utk, switchStkUtkMode(), mode=%d.", mode);
    // AT+EUTK
    char* cmd;
    err = asprintf(&cmd, "AT+EUTK=%d", mode);
    LOGD("ril_utk, switchStkUtkMode(), send command %s.", cmd);
    err = at_send_command(cmd, &p_response, STK_CHANNEL_CTX);
    free(cmd);
    at_response_free(p_response);
    p_response = NULL;
    return err;
}
/// M: [C2K][IR] Support CT 3g dual Mode card IR feature. @}

void requestSetUtkMode(void *data, size_t datalen, RIL_Token t) {
    RIL_UTK_UNUSED_PARM(datalen);
    int err;
    int mode = ((int *) data)[0];

    // switch UTK/STK mode.
    err = switchStkUtkMode(mode, t);

    LOGD("requestSetUtkMode: mode=%d, err=%d.", mode, err);
    if (err == 0) {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }
}

//  Add for BT SAP end
char* StkbtSapMsgIdToString(MsgId msgId) {
    switch (msgId) {
        case MsgId_RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS:
            return "BT_SAP_CARD_READER_STATUS";
        default:
            return "BT_SAP_UNKNOWN_MSG_ID";
    }
}

void sendStkBtSapResponseComplete(RIL_Token t, RIL_Errno ret, MsgId msgId, void *data) {
    const pb_field_t *fields = NULL;
    size_t encoded_size = 0;
    uint32_t written_size = 0;
    size_t buffer_size = 0;
    pb_ostream_t ostream;
    bool success = false;

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

        if (success) {
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
    RIL_UTK_UNUSED_PARM(data);
    RIL_UTK_UNUSED_PARM(datalen);
    RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ *req = NULL;
    // (RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ*)data;
    RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP rsp;
    BtSapStatus status = -1;

    LOGD("[BTSAP] requestBtSapGetCardStatus start, (%d)", rid);

    req = (RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ*)
        malloc(sizeof(RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ));
    assert(req != NULL);
    memset(req, 0, sizeof(RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ));

    status = queryBtSapStatus(rid);
    LOGD("[BTSAP] requestBtSapGetCardStatus status : %d", status);

    //  decodeStkBtSapPayload(MsgId_RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS, data, datalen, req);
    memset(&rsp, 0, sizeof(RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP));
    rsp.CardReaderStatus = BT_SAP_CARDREADER_RESPONSE_DEFAULT;
    rsp.response = RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_Response_RIL_E_SUCCESS;
    rsp.has_CardReaderStatus = true;   //  always true

    if (isSimInserted(rid)) {
        LOGD("[BTSAP] requestBtSapGetCardStatus, Sim inserted");
        rsp.CardReaderStatus = rsp.CardReaderStatus | BT_SAP_CARDREADER_RESPONSE_SIM_INSERT;
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
    if (!pb_decode(&stream, fields, dst)) {
        RLOGE("[BTSAP] decodeStkBtSapPayload, Error decoding protobuf buffer : %s", PB_GET_ERROR(&stream));
    } else {
        RLOGD("[BTSAP] decodeStkBtSapPayload, Success!");
    }
}

extern int rilStkBtSapMain(int request, void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid) {
    RIL_UTK_UNUSED_PARM(data);
    RIL_UTK_UNUSED_PARM(datalen);
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
// Add for BT SAP end

