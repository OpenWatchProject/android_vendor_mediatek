/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RilOemClient.h"
#include "RfxLog.h"
#include <sys/types.h>
#include <sys/socket.h>
#include "rfx_properties.h"
#include "RfxMessageId.h"
#include "RfxRilUtils.h"
#include "RfxDispatchThread.h"
#include "Rfx.h"
#include <netinet/in.h>
/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

#define RFX_LOG_TAG "RilOemClient"
#define OEM_MAX_PARA_LENGTH 768

#define RIL_OEM_SOCKET_COUNT 32
#define RIL_OEM_SOCKET_LENGTH 1024

RilOemClient::RilOemClient(int identity, char* socketName) : RilClient(identity, socketName) {
    RFX_LOG_I(RFX_LOG_TAG, "Init Oem client");
    mPhantomPacket = "";
}

RilOemClient::~RilOemClient() {

}

void RilOemClient::handleStateActive() {
    int number = 0;
    char **args;

    if (recv(commandFd, &number, sizeof(int), 0) != sizeof(int)) {
        RFX_LOG_E(RFX_LOG_TAG, "error reading on socket (number)");
        setClientState(CLIENT_DEACTIVE); /* or should set to init state*/
        return;
    }

    RFX_LOG_D(RFX_LOG_TAG, "NUMBER:%d", number);
    if (number < 0 || number > RIL_OEM_SOCKET_COUNT) {
        RFX_LOG_E(RFX_LOG_TAG, "Not allow negative number or more than limitation");
        setClientState(CLIENT_DEACTIVE);
        return;
    }
    args = (char **) calloc(1, sizeof(char*) * number);
    if (args == NULL) {
        RFX_LOG_E(RFX_LOG_TAG,"OOM");
        return;
    }

    for (int i = 0; i < number; i++) {
        unsigned int len;
        if (recv(commandFd, &len, sizeof(int), 0) != sizeof(int)) {
            RFX_LOG_E(RFX_LOG_TAG, "error reading on socket (Args)");
            freeArgs(args, number);
            setClientState(CLIENT_DEACTIVE); /* or should set to init state*/
            return;
        }
        // +1 for null-term

        if (len > RIL_OEM_SOCKET_LENGTH) {
            RFX_LOG_E(RFX_LOG_TAG, "lenght of args is less than 0 or more than limitation");
            freeArgs(args, number);
            setClientState(CLIENT_DEACTIVE);
            return;
        }
        RFX_LOG_D(RFX_LOG_TAG, "arg len:%u", len);
        args[i] = (char *) calloc(1, (sizeof(char) * len) + 1);
        if (args == NULL) {
            RFX_LOG_E(RFX_LOG_TAG,"OOM");
            return;
        }
        if (recv(commandFd, args[i], sizeof(char) * len, 0)
                != (int)(sizeof(char) * len)) {
            RFX_LOG_E(RFX_LOG_TAG, "error reading on socket: Args[%d] \n", i);
            freeArgs(args, number);
            setClientState(CLIENT_DEACTIVE); /* or should set to init state*/
            return;
        }
        char *buf = args[i];
        buf[len] = 0;

        RFX_LOG_D(RFX_LOG_TAG, "ARGS[%d]:%s", i, buf);
    }

    int result = handleSpecialRequestWithArgs(number, args);
    freeArgs(args, number);
    RFX_LOG_D(RFX_LOG_TAG, "result : %d", result);
    setClientState(CLIENT_DEACTIVE); /* or should set to init state*/
}

void RilOemClient::requestComplete(RIL_Token token, RIL_Errno e, void *response,
        size_t responselen) {
    RFX_UNUSED(responselen);
    RfxRequestInfo *info = (RfxRequestInfo *) token;
    if (RFX_MSG_REQUEST_QUERY_MODEM_THERMAL == info->request) {
        String8 strResult;
        RFX_LOG_D(RFX_LOG_TAG, "request for THERMAL returned");
        if(RIL_E_SUCCESS == e){
            strResult = String8((char*) response);
        } else {
            strResult = String8((char*) "ERROR");
        }

        if(mThermalFd >= 0){
            RFX_LOG_D(RFX_LOG_TAG, "mThermalFd is valid strResult is %s", strResult.string());

            size_t len = strResult.size();
            ssize_t ret = send(mThermalFd, strResult, len, MSG_NOSIGNAL);
            if (ret != (ssize_t) len) {
                RFX_LOG_D(RFX_LOG_TAG, "lose data when send response.");
            }
        } else {
            RFX_LOG_I(RFX_LOG_TAG, "mThermalFd is < 0");
        }
    } else if (RFX_MSG_REQUEST_RESET_RADIO == info->request) {
        if (commandFd >= 0) {
            close(commandFd);
        }
    } else if (RFX_MSG_REQUEST_QUERY_CAPABILITY == info->request) {
        if (mQueryCapFd >= 0) {
            int isEnabled = 0;
            if (response != NULL) {
                isEnabled = ((int*) response)[0];
            }
            RFX_LOG_D(RFX_LOG_TAG, "mQueryCapFd is valid ret is %d", isEnabled);
            int strResult = htonl(isEnabled);
            ssize_t ret = send(mQueryCapFd, (const char*) &strResult, 4, MSG_NOSIGNAL);
            if (ret != (ssize_t) sizeof(int)) {
                RFX_LOG_D(RFX_LOG_TAG, "lose data when send response.");
            }
        } else {
            RFX_LOG_I(RFX_LOG_TAG, "mQueryCapFd is < 0");
        }
    }
    setClientState(CLIENT_DEACTIVE); /* or should set to init state*/
    free(info);
}

void RilOemClient::handleUnsolicited(int slotId, int unsolResponse, void *data,
        size_t datalen, UrcDispatchRule rule) {
    RFX_UNUSED(slotId);
    RFX_UNUSED(unsolResponse);
    RFX_UNUSED(data);
    RFX_UNUSED(datalen);
    RFX_UNUSED(rule);
}

int RilOemClient::handleSpecialRequestWithArgs(int argCount, char** args) {
    char *cmd;
    char orgArgs[OEM_MAX_PARA_LENGTH] = {0};
    RfxAtLine *line;
    int err = 0;

    if (1 == argCount) {
        strncpy(orgArgs, args[0], OEM_MAX_PARA_LENGTH-1);
        line = new RfxAtLine(args[0], NULL);
        cmd = line->atTokNextstr(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "invalid command");
            delete line;
            return FAILURE;
        }

        if (strcmp(cmd, (char *) "THERMAL") == 0) {
            if (mThermalFd >= 0) {
                close(mThermalFd);
            }
            mThermalFd = commandFd;
            commandFd = -1;
            executeThermal(orgArgs);
            delete line;
            return SUCCESS;
        } else if (strcmp(cmd, "MDTM_TOG") == 0) {
            executeShutDownByThermal(orgArgs);
            delete line;
            return SUCCESS;
        } else if (strcmp(cmd, "MNGMT_PACKET") == 0) {
            executeUpdatePacket(orgArgs);
            delete line;
            return NO_ACTION;
        } else if (strcmp(cmd, "LOW_LATENCY_MODE") == 0) {
            executeLowLatencyMode(orgArgs);
            delete line;
            return NO_ACTION;
        } else if (strcmp(cmd, "GAME_MODE") == 0) {
            executeGameMode(orgArgs);
            delete line;
            return NO_ACTION;
         } else if (strcmp(cmd, "APP_STATUS") == 0) {
            executeAppStatus(orgArgs);
            delete line;
            return SUCCESS;
        } else if (strcmp(cmd, "QUERY_CAP") == 0) {
            executeQueryCapability(orgArgs);
            if (mQueryCapFd >= 0) {
                close(mQueryCapFd);
            }
            mQueryCapFd = commandFd;
            commandFd = -1;
            delete line;
            return SUCCESS;
        } else if (strcmp(cmd, "WEAK_SIGNAL_OPT") == 0) {
            executeWeakSignalOpt(orgArgs);
            delete line;
            return NO_ACTION;
        } else {
            delete line;
        }
    }
    RFX_LOG_E(RFX_LOG_TAG, "Invalid request");
    return FAILURE;
}

void RilOemClient::executeThermal(char *arg) {
    RFX_LOG_D(RFX_LOG_TAG, "executeThermal");
    char *cmd;
    int err = 0, slotId = 0;
    Parcel p;

    RfxAtLine *line = new RfxAtLine(arg, NULL);
    if (line->getCurrentLine() == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "executeThermal failed");
        delete line;
        return;
    }
    cmd = line->atTokNextstr(&err);
    slotId = line->atTokNextint(&err);
    int mainSlotId = RfxRilUtils::getMajorSim() - 1;
    RFX_LOG_D(RFX_LOG_TAG, "Thermal line = %s, cmd:%s, slotId:%d, targetSim: %d, arg : %s",
            arg, cmd, slotId, mainSlotId, line->getCurrentLine());

    RfxRequestInfo *pRI = (RfxRequestInfo *)calloc(1, sizeof(RfxRequestInfo));
    if (pRI == NULL) {
        RFX_LOG_E(RFX_LOG_TAG,"OOM");
        delete line;
        return;
    }
    pRI->socket_id = (RIL_SOCKET_ID) mainSlotId;
    pRI->token = 0xffffffff;
    pRI->clientId = (ClientId) CLIENT_ID_OEM;
    pRI->request = RFX_MSG_REQUEST_QUERY_MODEM_THERMAL;

    RFX_LOG_D(RFX_LOG_TAG, "arg : %s", line->getCurrentLine());
    rfx_enqueue_request_message_client(RFX_MSG_REQUEST_QUERY_MODEM_THERMAL,
            (void *) line->getCurrentLine(), strlen(line->getCurrentLine()), pRI,
            (RIL_SOCKET_ID) mainSlotId);
    delete line;
}

void RilOemClient::freeArgs(char** args, int number) {
    for (int i = 0; i < number; i++) {
        if (args[i]) {
            free(args[i]);
        }
    }
    if (args) {
        free(args);
    }
}

void RilOemClient::executeShutDownByThermal(char* arg) {
    RFX_LOG_D(RFX_LOG_TAG, "executeShutDownByThermal");
    char *cmd;
    int err = 0, modemOn = 0;
    Parcel p;

    RfxAtLine *line = new RfxAtLine(arg, NULL);
    cmd = line->atTokNextstr(&err);
    modemOn = line->atTokNextint(&err);
    int mainSlotId = RfxRilUtils::getMajorSim() - 1;
    RFX_LOG_D(RFX_LOG_TAG, "Thermal line = %s, cmd:%s, modemOn:%d, targetSim: %d",
            arg, cmd, modemOn, mainSlotId);

    RfxRequestInfo *pRI = (RfxRequestInfo *)calloc(1, sizeof(RfxRequestInfo));
    if (pRI == NULL) {
        RFX_LOG_E(RFX_LOG_TAG,"OOM");
        delete line;
        return;
    }
    pRI->socket_id = (RIL_SOCKET_ID) mainSlotId;
    pRI->token = 0xffffffff;
    pRI->clientId = (ClientId) CLIENT_ID_OEM;
    if (modemOn) {
        pRI->request = RFX_MSG_REQUEST_MODEM_POWERON;
    } else {
        pRI->request = RFX_MSG_REQUEST_MODEM_POWEROFF;
    }

    RFX_LOG_D(RFX_LOG_TAG, "arg : %s", line->getCurrentLine());
    rfx_enqueue_request_message_client(pRI->request, NULL, 0, pRI,
            (RIL_SOCKET_ID) mainSlotId);
    delete line;
}

void RilOemClient::executeGameMode(char *arg) {
    RFX_LOG_I(RFX_LOG_TAG, "executeGameModeCommand");
    char *cmd = NULL;
    int err = 0, gameMode = 0, lowLatencyMode = 0;
    char cmdStringOn[] = "AT+EGCMD=319, 6, \"000004B0FFFF\"";
    char cmdStringOff[] = "AT+EGCMD=319, 6, \"000000000000\"";

    RfxAtLine *line = new RfxAtLine(arg, NULL);
    cmd = line->atTokNextstr(&err);
    gameMode = line->atTokNextint(&err);
    lowLatencyMode = line->atTokNextint(&err);
    int mainSlotId = RfxRilUtils::getMajorSim() - 1;
    RFX_LOG_I(RFX_LOG_TAG, "executeGameMode line = %s, cmd: %s, gameMode: %d, lowLatencyMode: %d",
              arg, cmd, gameMode, lowLatencyMode);

    RfxRequestInfo *pRI = (RfxRequestInfo *)calloc(1, sizeof(RfxRequestInfo));
    if (pRI == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "OOM");
        delete line;
        return;
    }
    pRI->socket_id = (RIL_SOCKET_ID)mainSlotId;
    pRI->token = 0xffffffff;
    pRI->clientId = (ClientId)CLIENT_ID_OEM;
    pRI->request = RFX_MSG_REQUEST_OEM_HOOK_RAW;
    if (gameMode != -1) {
        mGameMode = gameMode;
    }
    if (lowLatencyMode != -1) {
        mLowLatencyMode = lowLatencyMode;
    }
    if (mGameMode == GAME_MODE_BATTLE) {
        RfxRilUtils::triggerPhantomPacket(mPhantomPacket);
        RFX_LOG_I(RFX_LOG_TAG, "arg : %s", cmdStringOn);
        rfx_enqueue_request_message_client(RFX_MSG_REQUEST_OEM_HOOK_RAW,
                (void *)cmdStringOn, strlen(cmdStringOn), pRI,
                (RIL_SOCKET_ID)mainSlotId);
    } else {
        RFX_LOG_I(RFX_LOG_TAG, "arg : %s", cmdStringOff);
        rfx_enqueue_request_message_client(RFX_MSG_REQUEST_OEM_HOOK_RAW,
                (void *)cmdStringOff, strlen(cmdStringOff), pRI,
                (RIL_SOCKET_ID)mainSlotId);
    }
    delete line;
}

void RilOemClient::executeUpdatePacket(char *arg) {
    RFX_LOG_D(RFX_LOG_TAG, "executeUpdatePacket");
    char *cmd = NULL;
    char *packet = NULL;
    int err = 0, length = 0;

    RfxAtLine *line = new RfxAtLine(arg, NULL);
    cmd = line->atTokNextstr(&err);
    packet = line->atTokNextstr(&err);
    RFX_LOG_D(RFX_LOG_TAG, "executeUpdatePacket line = %s, cmd: %s, packet: %s",
              arg, cmd, packet);

    mPhantomPacket = packet;
    RfxRilUtils::triggerPhantomPacket(mPhantomPacket);
    delete line;
}

void RilOemClient::executeLowLatencyMode(char *arg) {
    RFX_LOG_D(RFX_LOG_TAG, "executeLowLatencyModeCommand");
    char *cmd = NULL;
    int err = 0, lowLatencyMode = 0, period = 0;
    char cmdString3aOn[40] = "AT+EGCMD=319, 6,";
    char cmdString3aOff[] = "AT+EGCMD=319, 6, \"000000000000\"";
    char cmdString1aOn[] = "AT+EGCMD=315,1,\"02\"";
    char cmdString1aOff[] = "AT+EGCMD=315,1,\"00\"";

    RfxAtLine *line = new RfxAtLine(arg, NULL);
    cmd = line->atTokNextstr(&err);
    lowLatencyMode = line->atTokNextint(&err);
    period = line->atTokNextint(&err);
    int mainSlotId = RfxRilUtils::getMajorSim() - 1;
    RFX_LOG_D(RFX_LOG_TAG, "executeLowLatencyMode line = %s, cmd: %s, mode: %d, period: %d",
              arg, cmd, lowLatencyMode, period);

    RfxRequestInfo *pRI1a = (RfxRequestInfo *)calloc(1, sizeof(RfxRequestInfo));
    if (pRI1a == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "OOM");
        delete line;
        return;
    }

    pRI1a->socket_id = (RIL_SOCKET_ID)mainSlotId;
    pRI1a->token = 0xffffffff;
    pRI1a->clientId = (ClientId)CLIENT_ID_OEM;
    pRI1a->request = RFX_MSG_REQUEST_OEM_HOOK_RAW;

    RfxRequestInfo *pRI3a = (RfxRequestInfo *)calloc(1, sizeof(RfxRequestInfo));
    if (pRI3a == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "OOM");
        free(pRI1a);
        delete line;
        return;
    }

    pRI3a->socket_id = (RIL_SOCKET_ID)mainSlotId;
    pRI3a->token = 0xffffffff;
    pRI3a->clientId = (ClientId)CLIENT_ID_OEM;
    pRI3a->request = RFX_MSG_REQUEST_OEM_HOOK_RAW;

    if (lowLatencyMode == 1) {
        char *tempParam = NULL;

        asprintf(&tempParam, " \"%08xFFFF\"", period+200);
        strncat(cmdString3aOn, tempParam, strlen(tempParam));
        free(tempParam);
        tempParam = NULL;

        //RfxRilUtils::triggerPhantomPacket(mPhantomPacket);
        RFX_LOG_D(RFX_LOG_TAG, "arg : %s, %s", cmdString3aOn, cmdString1aOn);
        rfx_enqueue_request_message_client(RFX_MSG_REQUEST_OEM_HOOK_RAW,
                (void *)cmdString3aOn, strlen(cmdString3aOn), pRI3a,
                (RIL_SOCKET_ID)mainSlotId);
        rfx_enqueue_request_message_client(RFX_MSG_REQUEST_OEM_HOOK_RAW,
                (void *)cmdString1aOn, strlen(cmdString1aOn), pRI1a,
                (RIL_SOCKET_ID)mainSlotId);
    } else {
        RFX_LOG_D(RFX_LOG_TAG, "arg : %s, %s", cmdString3aOff, cmdString1aOff);
        rfx_enqueue_request_message_client(RFX_MSG_REQUEST_OEM_HOOK_RAW,
                (void *)cmdString3aOff, strlen(cmdString3aOff), pRI3a,
                (RIL_SOCKET_ID)mainSlotId);
        rfx_enqueue_request_message_client(RFX_MSG_REQUEST_OEM_HOOK_RAW,
                (void *)cmdString1aOff, strlen(cmdString1aOff), pRI1a,
                (RIL_SOCKET_ID)mainSlotId);
    }
    delete line;
}

void RilOemClient::executeWeakSignalOpt(char *arg) {
    RFX_LOG_D(RFX_LOG_TAG, "executeWeakSignalOpt");
    char *cmd = NULL;
    int err = 0, enable = 0;
    char cmdString1aOn[] = "AT+EGCMD=315,1,\"02\"";
    char cmdString1aOff[] = "AT+EGCMD=315,1,\"00\"";

    RfxAtLine *line = new RfxAtLine(arg, NULL);
    cmd = line->atTokNextstr(&err);
    enable = line->atTokNextint(&err);
    int mainSlotId = RfxRilUtils::getMajorSim() - 1;
    RFX_LOG_D(RFX_LOG_TAG, "executeWeakSignalOpt line = %s, cmd: %s, enable: %d",
              arg, cmd, enable);

    RfxRequestInfo *pRI1a = (RfxRequestInfo *)calloc(1, sizeof(RfxRequestInfo));
    if (pRI1a == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "OOM");
        delete line;
        return;
    }

    pRI1a->socket_id = (RIL_SOCKET_ID)mainSlotId;
    pRI1a->token = 0xffffffff;
    pRI1a->clientId = (ClientId)CLIENT_ID_OEM;
    pRI1a->request = RFX_MSG_REQUEST_OEM_HOOK_RAW;

    if (enable == 1) {
        RFX_LOG_D(RFX_LOG_TAG, "arg : %s", cmdString1aOn);
        rfx_enqueue_request_message_client(RFX_MSG_REQUEST_OEM_HOOK_RAW,
                (void *)cmdString1aOn, strlen(cmdString1aOn), pRI1a,
                (RIL_SOCKET_ID)mainSlotId);
    } else {
        RFX_LOG_D(RFX_LOG_TAG, "arg : %s", cmdString1aOff);
        rfx_enqueue_request_message_client(RFX_MSG_REQUEST_OEM_HOOK_RAW,
                (void *)cmdString1aOff, strlen(cmdString1aOff), pRI1a,
                (RIL_SOCKET_ID)mainSlotId);
    }

    delete line;
}

void RilOemClient::executeAppStatus(char *arg) {
    char *cmd = NULL;
    int err = 0, pid = 0, status = 0;

    RfxAtLine *line = new RfxAtLine(arg, NULL);
    cmd = line->atTokNextstr(&err);
    pid = line->atTokNextint(&err);
    status = line->atTokNextint(&err);
    int mainSlotId = RfxRilUtils::getMajorSim() - 1;
    RFX_LOG_I(RFX_LOG_TAG, "executeAppStatus line = %s, cmd: %s, pid: %d, status: %d",
              arg, cmd, pid, status);
    if (STATE_DEAD == status) {
        RfxRequestInfo *pRI = (RfxRequestInfo *)calloc(1, sizeof(RfxRequestInfo));
        if (pRI == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "OOM");
            delete line;
            return;
        }
        pRI->socket_id = (RIL_SOCKET_ID)mainSlotId;
        pRI->token = 0xffffffff;
        pRI->clientId = (ClientId)CLIENT_ID_OEM;
        pRI->request = RFX_MSG_REQUEST_ABORT_CERTIFICATE;
        rfx_enqueue_request_message_client(RFX_MSG_REQUEST_ABORT_CERTIFICATE,
                (void *)&pid, sizeof(int), pRI, (RIL_SOCKET_ID)mainSlotId);
    } else {
        RFX_LOG_E(RFX_LOG_TAG, "executeAppStatus unexpected status: %d", status);
    }

    delete line;
}

void RilOemClient::executeQueryCapability(char* arg) {
    char *cmd = NULL;
    int err = 0, pid = 0, status = 0;
    char *featureName;

    RfxAtLine *line = new RfxAtLine(arg, NULL);
    cmd = line->atTokNextstr(&err);
    pid = line->atTokNextint(&err);
    featureName = line->atTokNextstr(&err);
    int mainSlotId = RfxRilUtils::getMajorSim() - 1;
    RFX_LOG_I(RFX_LOG_TAG, "executeQueryCapability cmd: %s, pid: %d, name: %s",
              cmd, pid, featureName);

    // create RequestInfo and send to TelephonyWare
    RfxRequestInfo *pRI = (RfxRequestInfo *)calloc(1, sizeof(RfxRequestInfo));
    if (pRI == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "OOM");
        delete line;
        return;
    }
    pRI->socket_id = (RIL_SOCKET_ID)mainSlotId;
    pRI->token = 0xffffffff;
    pRI->clientId = (ClientId)CLIENT_ID_OEM;
    pRI->request = RFX_MSG_REQUEST_QUERY_CAPABILITY;

    RIL_QueryCap *msg = (RIL_QueryCap *) calloc(1, sizeof(RIL_QueryCap));
    if (msg == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "OOM");
        free(pRI);
        delete line;
        return;
    }
    msg->id = pid;
    msg->name = strdup(featureName);

    rfx_enqueue_request_message_client(RFX_MSG_REQUEST_QUERY_CAPABILITY,
            (void *)msg, sizeof(RIL_QueryCap), pRI, (RIL_SOCKET_ID)mainSlotId);
    delete line;
    free(msg->name);
    free(msg);
}