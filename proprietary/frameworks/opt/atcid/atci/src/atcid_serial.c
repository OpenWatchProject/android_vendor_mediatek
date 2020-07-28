/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include "atcid.h"
#include "atcid_serial.h"
#include "atcid_cmd_dispatch.h"
#include "atcid_cust_cmd.h"
#include "atcid_util.h"
#include "at_tok.h"
#include "atcid_adaptation.h"
#include <string.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <netinet/in.h>
#include <termios.h>
#include "property/mtk_properties.h"
#include <signal.h>
#include <regex.h>
#include <unistd.h>


#define MAX_FAILURE_RETRY 3
#define BUILD_TYPE_PROP "ro.build.type"
#define BUILD_TYPE_PROP_ENG "eng"
#define BUILD_TYPE_PROP_USER "user"
#define IS_EQUAL(a,b) ((strcmp(a,b)==0)?true:false)
#define MAX(a,b) ((a)>(b)?(a):(b))


/*misc global vars */
extern Serial serial;
extern At_Ril_Mmi_t atRilMmi_info;

#define CONN_VCOM 1
#define CONN_ADB 2
#define CONN_HIDL 3

int s_fdRild_command = INVALIDE_SOCKET_FD;
int s_fdService_command = INVALIDE_SOCKET_FD;
int s_fdService_command_vendor = INVALIDE_SOCKET_FD;
int s_fdAudio_command = INVALIDE_SOCKET_FD;
int s_fdAudioExt_command = INVALIDE_SOCKET_FD;
int s_fdADBSK_client = INVALIDE_SOCKET_FD;
int s_fdADBSK_server = INVALIDE_SOCKET_FD;
int atci_conn_mode = CONN_VCOM;
int s_fdMeta_command = INVALIDE_SOCKET_FD;

int mmiCmdSeqEnQueue(int idx);
int mmiCmdSeqDeQueue();
int getMsgCmdParameterCount(char* line);
int startMergeMsgCmdInputIfNeed(Serial *serial, char* line);

SocketId connc_socket = SOCKET_MD1;
bool isNeedSwitchSocket = false;
bool isRilAtciSocketExist = true;
bool isATCmdInject = false;
bool isMsgCmdInput = false;
bool isNeedMergeMsgCmdInput = false;
int msgCmdMode = 0;
bool isNumericFormat = false;
bool isLastResponseOk = false;

#define ATCI_SIM "persist.vendor.service.atci.sim"
#define IS_RILDFDVALID() ((s_fdRild_command > 0)&&(true == isRilAtciSocketExist)? true:false)
#define NOMODEM_RSP "ERROR:No Modem"
#define NOUE_RSP "ERROR:UE is not ready"
#define NORIL_RSP "Not access to modem in User Mode."
#define INTERMIDIATE_RSP "\r\n> "
#define META_CFG_CMD "AT+EMETACFG=1"
#define META_REQUEST "calibration"
#define META_RESPONSE "calibration_done"

void sigpipe_handler(int s) {
    UNUSED(s);
    LOGATCI(LOG_ERR, "Caught SIGPIPE\n");
}


#define CTRLCHAR_SUB 0x1a
#define CTRLCHAR_ESC 0x1b
#define CTRLCHAR_LF 0x0a

bool check_SUB_ESC_char(char* line) {
    char *pLine = line;

    while(*pLine != '\0') {
        int currChar = (int)*pLine;
        if(CTRLCHAR_SUB == currChar || CTRLCHAR_ESC == currChar) {
            return true;
        }
        pLine++;
    }
    return false;
}

int sendDataToRild(char* line) {
    int sendLen = 0, len = 0;
    LOGATCI(LOG_DEBUG,"Enter");
    len = strlen(line);
    uint32_t header = htonl(len);
    send(s_fdRild_command, (void *)&header, sizeof(uint32_t), 0);
    sendLen = send(s_fdRild_command, line, len, 0);
    isATCmdInject = true;
    isLastResponseOk = false;
    if (sendLen != len) {
        LOGATCI(LOG_ERR, "lose data when send to ril. errno = %d, sendLen = %d", errno, sendLen);
    }
    LOGATCI(LOG_DEBUG," send data over");
    if (check_SUB_ESC_char(line)) {
        LOGATCI(LOG_DEBUG," SUB or ESC in line, clear isMsgCmdInput");
        isMsgCmdInput = false;
    }
    return 0;
}

void writeDataToserialByResponseType(ATRESPONSE_t response_type) {
    char response[MAX_AT_RESPONSE];
    int i = 0;

    memset(response, 0, sizeof(response));

    response[i++] = AT_CR;
    response[i++] = AT_LF;

    switch(response_type) {
        case AT_OK:
            memcpy(response+i, AT_OK_STRING, strlen(AT_OK_STRING));
            i+=strlen(AT_OK_STRING);
            break;
        case AT_CONNECT:
            break;
        case AT_ERROR:
            memcpy(response+i, AT_ERROR_STRING, strlen(AT_ERROR_STRING));
            i+=strlen(AT_ERROR_STRING);
            break;
        case AT_NOT_IMPL:
            memcpy(response+i, AT_NOT_IMPL_STRING, strlen(AT_NOT_IMPL_STRING));
            i+=strlen(AT_NOT_IMPL_STRING);
            break;
        default:
            memcpy(response+i, AT_UNKNOWN_STRING, strlen(AT_UNKNOWN_STRING));
            i+=strlen(AT_UNKNOWN_STRING);
            break;
    }
    response[i++] = AT_CR;
    response[i++] = AT_LF;

    writeDataToSerial(response, strlen(response));
}

void writeDataToSerialByDataType(ATCI_DataType dataType, char* input, int length) {
    int c = 0;
    int fd = INVALIDE_SOCKET_FD;
    char buffer[length + 1];
    char response[length + 1];
    char output[length + 5];
    bool convertedToNumeric = false;

    LOGATCI(LOG_DEBUG,"Enter with dataType:%d", dataType);
    LOGATCI(LOG_DEBUG,"Enter with data:%s", input);
    LOGATCI(LOG_DEBUG,"Enter with length:%d", length);

    if(ADB_SKT_SERVER_NUM  == serial.currDevice)
        fd = serial.fd[ADB_SKT_CLIENT_NUM];
    else
        fd = serial.fd[serial.currDevice];

    memset(buffer, 0, sizeof(buffer));
    memset(response, 0, sizeof(response));
    memset(output, 0, sizeof(output));

    if (fd >=0) {
        /*audio type response must be 1 of 4 prefix above,
          audio response is not AT command, should not be add <CR><LF> here*/
        if (!(strncmp(input, "AUD", 3) == 0 ||
              strncmp(input, "aud", 3) == 0 ||
              strncmp(input, "MM", 2) == 0 ||
              strncmp(input, "mm", 2) == 0 )) {
            // remove <CR><LF> head and tail
            LOGATCI(LOG_DEBUG,"Not audio");
            if (strncmp(input, "\r\n", 2) == 0) {
                if (strncmp((input + length - 2), "\r\n", 2) == 0) {
                    memcpy(buffer, input + 2, length - 4);
                    LOGATCI(LOG_DEBUG,"remove head and tail");
                } else {
                    memcpy(buffer, input + 2, length - 2);
                    LOGATCI(LOG_DEBUG,"remove head");
                }
            } else if (strncmp((input+length - 2), "\r\n", 2) == 0) {
                memcpy(buffer, input, length - 2);
                LOGATCI(LOG_DEBUG,"remove tail");
            } else {
                memcpy(buffer, input, length);
                LOGATCI(LOG_DEBUG,"No remove");
            }

            if (strncmp(buffer, "OK", strlen("OK")) == 0) {
                isLastResponseOk = true;
            } else {
                isLastResponseOk = false;
            }

            // Convert to response
            if (isNumericFormat) {
                convertedToNumeric = Response2Numeric(buffer, response);
            } else {
                memcpy(response, buffer, sizeof(buffer));
            }

            if (dataType == RIL_TYPE && !convertedToNumeric) {
                // Output strings from rild as-is
                memcpy(output, input, length);
            } else {
                // Add <CR><LF> head and tail
                LOGATCI(LOG_DEBUG, "add head and tail");
                sprintf(output, "\r\n%s\r\n", response);
                length = strlen(response) + 4;
            }
        } else {
            LOGATCI(LOG_INFO,"audio type response");
            memcpy(output, input, length);
        }

        LOGATCI(LOG_DEBUG,"length=%d", length);
        if (HIDL_CONN_NUM == serial.currDevice) {
            sendResponse(output, length);
        } else {
            c = write(fd, output, length);
        }

        if (length != c) {
            LOGATCI(LOG_WARNING, "Couldn't write all data to the serial port. Wrote only %d bytes with %d", c, errno);
        }
    } else {
        LOGATCI(LOG_ERR, "Wrong fd with %d, currDevice = %d", fd, serial.currDevice);
    }

    return;
}

void writeDataToSerial(char* input, int length) {
    writeDataToSerialByDataType(UNKNOWN_TYPE, input, length);
}

int sendDataToGenericService(char* line) {
    int len = 0;
    LOGATCI(LOG_DEBUG,"Enter");

    len = strlen(line);
    int sendLen = send(s_fdService_command_vendor, line, len, 0);
    if (sendLen != len) {
        LOGATCI(LOG_ERR, "lose data when send to generic service. errno = %d", errno);
    }
    sendCommandToSystemAtciService(line, len);
    return 0;
}

int sendDataToAudio(char* line, int len, ATCI_DataType dataType) {
    int sendLen = 0;
    int fd = INVALIDE_SOCKET_FD;
    LOGATCI(LOG_DEBUG,"Enter");

    if (AUDIO_TYPE == dataType) {
        fd = s_fdAudio_command;
    } else {
        LOGATCI(LOG_ERR,"invalid audio data type.");
        return 0;
    }
    if (fd < 0) {
        LOGATCI(LOG_ERR,"invalid fd [%d].",fd);
        return 0;
    }
    LOGATCI(LOG_WARNING, "sendDataToAudio - the data size of send to audio is: %d", len);
    sendLen = send(fd, line, len, 0);
    if (sendLen != len) {
        LOGATCI(LOG_ERR, "lose data when send to audio. errno = %d", errno);
    }

    return 0;
}

int sendDataToMeta(char* line) {
    int sendLen = 0;
    int fd = s_fdMeta_command;
    LOGATCI(LOG_DEBUG, "Enter");

    if (fd < 0) {
        LOGATCI(LOG_ERR, "invalid fd [%d].", fd);
        return -1;
    }
    int len = strlen(line);
    LOGATCI(LOG_INFO, "sendDataToMeta - the data size of send to meta is: %d", len);
    sendLen = send(fd, line, len, 0);
    LOGATCI(LOG_INFO, "sendDataToMeta - the data size of sent to meta is: %d, errno is: %d", sendLen, errno);
    if (sendLen < 0) {
        LOGATCI(LOG_ERR, "Error when sending data. errno = %d", errno);
        close(s_fdMeta_command);
        s_fdMeta_command = -1;
        return -1;
    }

    return 0;
}

void readDataFromTargetWithResult(ATCI_DataType dataType, int* s_fd_listen, char* buffer, int buffer_size) {
    int recvLen = 0;

    LOGATCI(LOG_DEBUG,"Enter");
    LOGATCI(LOG_INFO,"Read data from target:%d with fd:%d", dataType, *s_fd_listen);

    memset(buffer, 0, buffer_size);

    do {
        recvLen = recv(*s_fd_listen, buffer, MAX_DATA_SIZE, 0);
        if (recvLen == -1) {
            if(errno == EAGAIN || errno == EINTR) {
                continue;
            }
            LOGATCI(LOG_ERR, "fail to receive data from target socket. errno = %d", errno);
            close(*s_fd_listen);
            *s_fd_listen = INVALIDE_SOCKET_FD;
            return;
        } else if(recvLen == 0) {
            LOGATCI(LOG_ERR, "The peer has performed an orderly shutdown");
            close(*s_fd_listen);
            *s_fd_listen = INVALIDE_SOCKET_FD;
            return;
        }
    } while(recvLen <= 0);

    LOGATCI(LOG_INFO, "data receive from %d is %s, data length is %d", dataType, buffer, recvLen);
    writeDataToSerial(buffer, recvLen);
}

void sendCusMMICmdRspToMMI(char* rsp) {
    char* pPackRsp = NULL;
    char* cmd = NULL;

    LOGATCI(LOG_DEBUG, "sendCusMMICmdRspToMMI SIdx[%d]", atRilMmi_info.cmdNameSIdx);
    if (NULL == rsp) {
        return;
    }

    cmd = atRilMmi_info.cmdName[atRilMmi_info.cmdNameSIdx];
    pPackRsp = (char*)calloc(1, strlen(cmd) + strlen(rsp) + 2); // +2 for ';' and '\0'
    strncpy(pPackRsp, cmd, strlen(cmd));
    strncat(pPackRsp, rsp, strlen(rsp));
    strncat(pPackRsp, ";", 1);

    atRilMmi_info.cmdNameSIdx++;
    if (MAX_QUEUE_MMI_CMD_IDX <= atRilMmi_info.cmdNameSIdx) {
        atRilMmi_info.cmdNameSIdx = 0;
    }

    if (s_fdService_command < 0) {
        connectTarget(GENERIC_TYPE);
    }
    sendDataToGenericService(pPackRsp);
    free(pPackRsp);
}

void readDataFromTarget(ATCI_DataType dataType, int* s_fd_listen) {
    int recvLen = 0;
    char buffer[MAX_DATA_SIZE + 1];

    LOGATCI(LOG_DEBUG,"Enter");
    LOGATCI(LOG_INFO,"Read data from target:%d with fd:%d", dataType, *s_fd_listen);

    memset(buffer, 0, sizeof(buffer));

    do {
        recvLen = recv(*s_fd_listen, buffer, MAX_DATA_SIZE, 0);
        if (recvLen == -1) {
            if(errno == EAGAIN || errno == EINTR) {
                continue;
            }
            LOGATCI(LOG_ERR, "fail to receive data from target socket. errno = %d", errno);
            close(*s_fd_listen);
            *s_fd_listen = INVALIDE_SOCKET_FD;
            return;
        } else if(recvLen == 0) {
            LOGATCI(LOG_ERR, "The peer has performed an orderly shutdown, isATCmdInject %d",isATCmdInject);
            close(*s_fd_listen);
            *s_fd_listen = INVALIDE_SOCKET_FD;
            if(true == isATCmdInject) {
                writeDataToSerial(NOUE_RSP, strlen(NOUE_RSP)+1);
                isATCmdInject = false;
            }
            return;
        }
    } while(recvLen <= 0);

    if(RIL_TYPE == dataType) {
        LOGATCI(LOG_DEBUG, "Receive RIL rsp = [%d]", atRilMmi_info.cmdSeq);
        atRilMmi_info.cmdSeq--;//Receive response of a command from RILD

        if(1 == mmiCmdSeqDeQueue()) {
            sendCusMMICmdRspToMMI(buffer);
        }
    }

    isATCmdInject = false;
    LOGATCI(LOG_INFO, "data receive from %d is %s", dataType, buffer);
    LOGATCI(LOG_INFO, "data receive data length is %d", recvLen);
    writeDataToSerialByDataType(dataType, buffer, recvLen);

    if (isMsgResponse(buffer)) {
        isMsgCmdInput = false;
    }
}

bool isCdmaLteDcSupport() {
    int isC2kLteSupport = 0;
    bool ret = false;

    char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
    mtk_property_get("ro.vendor.mtk_c2k_lte_mode", property_value, "0");
    isC2kLteSupport = atoi(property_value);

    LOGATCI(LOG_DEBUG, "isC2kLteSupport: %d", isC2kLteSupport);
    if (isC2kLteSupport != 0) {
        ret = true;
    }

    return ret;
}

void connectTarget(ATCI_DataType dataType) {
    LOGATCI(LOG_DEBUG,"Enter");

    if (RIL_TYPE == dataType) {
        if (s_fdRild_command == INVALIDE_SOCKET_FD) {
            char prop[MTK_PROPERTY_VALUE_MAX] = {0};
            mtk_property_get("ro.vendor.mtk_ril_mode", prop, "");
            LOGATCI(LOG_DEBUG, "ro.vendor.mtk_ril_mode %s", prop);
            if (strcmp(prop, "c6m_1rild") == 0) {
                s_fdRild_command = socket_local_client(SOCKET_NAME_RILD,
                                                   ANDROID_SOCKET_NAMESPACE_RESERVED,
                                                   SOCK_STREAM);
            } else {
                s_fdRild_command = socket_local_client(SOCKET_NAME_RILPROXY,
                                                   ANDROID_SOCKET_NAMESPACE_RESERVED,
                                                   SOCK_STREAM);
            }

            if (s_fdRild_command < 0) {
                if (errno == ENOENT) { // No such file or directory
                    ALOGE("ENOENT[%d] : No such file or directory",ENOENT);
                    isRilAtciSocketExist = false;
                }
                ALOGE("fail to open atci-ril socket. errno:%d", errno);
                close(s_fdRild_command);
                return;
            }
        }
    } else if (GENERIC_TYPE == dataType) {
        if (s_fdService_command == INVALIDE_SOCKET_FD) {
            s_fdService_command = connectToSystemAtciService();
        }
        if (s_fdService_command_vendor == INVALIDE_SOCKET_FD) {
            s_fdService_command_vendor = socket_local_client(SOCKET_NAME_GENERIC,
                                  ANDROID_SOCKET_NAMESPACE_RESERVED,
                                  SOCK_STREAM);

            if (s_fdService_command_vendor < 0) {
                ALOGE("fail to open atci generic socket. errno:%d", errno);
                close(s_fdService_command_vendor);
                return;
            }
        }
    } else if (AUDIO_TYPE == dataType) {
        s_fdAudio_command = socket_local_client(SOCKET_NAME_AUDIO,
                                                ANDROID_SOCKET_NAMESPACE_RESERVED,
                                                SOCK_STREAM);

        if (s_fdAudio_command < 0) {
            ALOGE("fail to open atci audio socket. errno:%d", errno);
            close(s_fdAudio_command);
            return;
        }
    } else if (AUDIO_EXTERNAL_TYPE == dataType) {
        s_fdAudioExt_command = socket_local_client(SOCKET_NAME_RILD2,
                               ANDROID_SOCKET_NAMESPACE_RESERVED,
                               SOCK_STREAM);

        if (s_fdAudioExt_command < 0) {
            ALOGE("fail to open atci audio external socket. errno:%d", errno);
            close(s_fdAudioExt_command);
            return;
        }
    } else if (META_TYPE == dataType && s_fdMeta_command < 0) {
        s_fdMeta_command = socket_local_client(SOCKET_NAME_META,
                                               ANDROID_SOCKET_NAMESPACE_RESERVED,
                                               SOCK_STREAM);
        if (s_fdMeta_command < 0) {
            ALOGE("fail to open meta-atci socket. errno:%d", errno);
            close(s_fdMeta_command);
            return;
        }
    }
}

int serviceReaderLoopWithResult(char* line) {
    UNUSED(line);
    int ret;
    fd_set rfds;
    char buffer[MAX_DATA_SIZE + 1];

    LOGATCI(LOG_DEBUG,"Enter");

    if(s_fdService_command != INVALIDE_SOCKET_FD) {
        FD_ZERO(&rfds);
        FD_SET(s_fdService_command, &rfds);

        ret = select(s_fdService_command+1, &rfds, NULL, NULL, NULL);
        if (ret < 0) {
            if(errno == EINTR || errno == EAGAIN) {
                return -1;
            }
            LOGATCI(LOG_ERR, "Fail to select in serviceReaderLoop. error: %d", errno);
            close(s_fdService_command);
            s_fdService_command = INVALIDE_SOCKET_FD;
            return -1;
        } else if(ret == 0) {
            LOGATCI(LOG_DEBUG, "timeout for select in serviceReaderLoop");
            close(s_fdService_command);
            s_fdService_command = INVALIDE_SOCKET_FD;
            return -1;
        }

        if (FD_ISSET(s_fdService_command, &rfds)) {
            LOGATCI(LOG_DEBUG, "Prepare read the data from generic service");
            readDataFromTargetWithResult(GENERIC_TYPE, &s_fdService_command, buffer, MAX_DATA_SIZE + 1);
            if (strstr(buffer, "OK") != NULL) {
                return 1;
            } else {
                return -1;
            }
        }
        return -1;
    }
    return -1;
}

void serviceReaderLoop() {
    int ret;
    fd_set rfds;

    LOGATCI(LOG_DEBUG,"Enter");

    if(s_fdService_command != INVALIDE_SOCKET_FD) {
        FD_ZERO(&rfds);
        FD_SET(s_fdService_command, &rfds);

        ret = select(s_fdService_command+1, &rfds, NULL, NULL, NULL);
        if (ret < 0) {
            if(errno == EINTR || errno == EAGAIN) {
                return;
            }
            LOGATCI(LOG_ERR, "Fail to select in serviceReaderLoop. error: %d", errno);
            close(s_fdService_command);
            s_fdService_command = INVALIDE_SOCKET_FD;
            return;
        } else if(ret == 0) {
            LOGATCI(LOG_DEBUG, "timeout for select in serviceReaderLoop");
            close(s_fdService_command);
            s_fdService_command = INVALIDE_SOCKET_FD;
            return;
        }

        if (FD_ISSET(s_fdService_command, &rfds)) {
            LOGATCI(LOG_DEBUG, "Prepare read the data from generic service");
            readDataFromTarget(GENERIC_TYPE, &s_fdService_command);
        }
    }
}

void rildReaderLoop() {
    int ret;
    fd_set rfds;

    LOGATCI(LOG_DEBUG,"Enter");

    if(s_fdRild_command != INVALIDE_SOCKET_FD) {
        FD_ZERO(&rfds);
        FD_SET(s_fdRild_command, &rfds);

        ret = select(s_fdRild_command+1, &rfds, NULL, NULL, NULL);
        if (ret < 0) {
            if(errno == EINTR || errno == EAGAIN) {
                return;
            }
            LOGATCI(LOG_ERR, "Fail to select in rildReaderLoop. error: %d", errno);
            close(s_fdRild_command);
            s_fdRild_command = INVALIDE_SOCKET_FD;
            return;
        } else if(ret == 0) {
            LOGATCI(LOG_DEBUG, "timeout for select in rildREaderLoop");
            close(s_fdRild_command);
            s_fdRild_command = INVALIDE_SOCKET_FD;
            return;
        }

        if (FD_ISSET(s_fdRild_command, &rfds)) {
            LOGATCI(LOG_DEBUG, "Prepare read the data from rild driver");
            readDataFromTarget(RIL_TYPE, &s_fdRild_command);
        }
    }
}

void audioReaderLoop(ATCI_DataType dataType) {
    int ret;
    fd_set rfds;
    int *fd = NULL;

    LOGATCI(LOG_DEBUG,"Enter");
    if (AUDIO_TYPE == dataType) {
        fd = &s_fdAudio_command;
    }
    if (fd == NULL) {
        LOGATCI(LOG_DEBUG,"invalid fd NULL");
        return;
    }
    if (*fd < 0) {
        LOGATCI(LOG_DEBUG,"invalid fd [%d]", *fd);
        return;
    }
    LOGATCI(LOG_DEBUG,"valid fd [%d]", *fd);

    FD_ZERO(&rfds);
    FD_SET(*fd, &rfds);

    ret = select(*fd + 1, &rfds, NULL, NULL, NULL);
    if (ret < 0) {
        if(errno == EINTR || errno == EAGAIN) {
            return;
        }
        LOGATCI(LOG_ERR, "Fail to select in audioReaderLoop. error: %d", errno);
        close(*fd);
        *fd = INVALIDE_SOCKET_FD;
        return;
    } else if(ret == 0) {
        LOGATCI(LOG_DEBUG, "timeout for select in audioReaderLoop");
        close(*fd);
        *fd = INVALIDE_SOCKET_FD;
        return;
    }

    if (FD_ISSET(*fd, &rfds)) {
        LOGATCI(LOG_DEBUG, "Prepare read the data from audio driver");
        readDataFromTarget(AUDIO_TYPE, fd);
    }
}

int metaReaderLoop() {
    int ret;
    fd_set rfds;
    char buffer[MAX_DATA_SIZE + 1];
    int recvLen = 0;
    LOGATCI(LOG_DEBUG, "Enter");

    if (s_fdMeta_command != INVALIDE_SOCKET_FD) {
        FD_ZERO(&rfds);
        FD_SET(s_fdMeta_command, &rfds);

        ret = select(s_fdMeta_command + 1, &rfds, NULL, NULL, NULL);
        if (ret < 0) {
            if (errno == EINTR || errno == EAGAIN) {
                return -1;
            }
            LOGATCI(LOG_ERR, "Fail to select in metaReaderLoop. error: %d", errno);
            close(s_fdMeta_command);
            s_fdMeta_command = INVALIDE_SOCKET_FD;
            return -1;
        } else if (ret == 0) {
            LOGATCI(LOG_DEBUG, "timeout for select in metaReaderLoop");
            close(s_fdMeta_command);
            s_fdMeta_command = INVALIDE_SOCKET_FD;
            return -1;
        }

        if (FD_ISSET(s_fdMeta_command, &rfds)) {
            memset(buffer, 0, sizeof(buffer));
            do {
                recvLen = recv(s_fdMeta_command, buffer, MAX_DATA_SIZE, 0);
                if (recvLen == -1) {
                    if (errno == EAGAIN || errno == EINTR) {
                        continue;
                    }
                    LOGATCI(LOG_ERR, "fail to receive data from target socket. errno = %d", errno);
                    close(s_fdMeta_command);
                    s_fdMeta_command = INVALIDE_SOCKET_FD;
                    return -1;
                } else if (recvLen == 0) {
                    LOGATCI(LOG_ERR, "The peer has performed an orderly shutdown");
                    close(s_fdMeta_command);
                    s_fdMeta_command = INVALIDE_SOCKET_FD;
                    return -1;
                }
            } while (recvLen <= 0);

            buffer[MAX_DATA_SIZE] = '\0';
            LOGATCI(LOG_INFO, "Received %s", buffer);
            if (strcmp(buffer, META_RESPONSE) == 0) {
                return 0;
            }
        }
    }

    return -1;
}

void suspendAtciForMeta() {
    int ret = -1;
    close(serial.fd[serial.currDevice]);
    ret = sendDataToMeta(META_REQUEST);
    if (ret < 0) {
        connectTarget(META_TYPE);
        ret = sendDataToMeta(META_REQUEST);
    }
    if (ret >= 0) {
        while (metaReaderLoop() < 0) {
            LOGATCI(LOG_ERR, "Error when receiving from meta");
            connectTarget(META_TYPE);
            usleep(1000 * 1000);
        }
    }
    serial.fd[serial.currDevice] = open_serial_device(
            &serial, serial.devicename[serial.currDevice]);
}

#define CTRLCHAR_BS 0x08
#define CTRLCHAR_DEL 0x7F

void processCtrlChar_BackSpace(char* line) {
    char buf[MAX_DATA_SIZE + 1] = {0};
    char *pLine = line;
    int currLineIdx = 0;

    while(*pLine != '\0') {
        int currChar = (int)*pLine;
        if (CTRLCHAR_BS == currChar || CTRLCHAR_DEL == currChar) {
            if(0 < currLineIdx)
                currLineIdx--;//Move back one char
        } else {
            buf[currLineIdx] = *pLine;
            currLineIdx++;
        }
        pLine++;
    }
    strncpy(line, buf, strlen(buf));
    line[currLineIdx] = '\0';
}
/**
 * Reads a line from the AT channel, returns NULL on timeout.
 * Assumes it has exclusive read access to the FD
 *
 * This line is valid only until the next call to readline
 *
 * This function exists because as of writing, android libc does not
 * have buffered stdio.
 */

static const char *readline(Serial *serial) {
    fd_set rfds;
    char *p_read = NULL;
    int ret = 0;
    char buffer[MAX_DATA_SIZE + 1] = {0};
    char tmpBuffer[MAX_DATA_SIZE + 1] = {0};
    char *p_buffer = buffer;
    int readCurr = 0, readCount = 0, tmp_readCurr = 0;
    int err = 0, i = 0;
    int fd = INVALIDE_SOCKET_FD, currDevice = 0;
    int headerSize = 0;

    LOGATCI(LOG_DEBUG,"Enter");

    memset(buffer, 0, sizeof(buffer));
    memset(tmpBuffer, 0, sizeof(tmpBuffer));
    atci_conn_mode = CONN_VCOM;
    //Start to read the command data from USB devices
    for(;;) {

        //Initial the default value for each local paramters
        //Set the buffer data to zero
        if (!isNeedMergeMsgCmdInput) {
            memset(serial->ATBuffer, 0, sizeof(serial->ATBuffer));
            serial->totalSize = 0;
        }
        if (CONN_VCOM == atci_conn_mode) {
            memset(buffer, 0, sizeof(buffer));
        }
        serial->at_state = FIND_A;      //Inital state to find 'A' character in command line
        readCurr = 0;
        FD_ZERO(&rfds);

        for(i = 0; i < MAX_DEVICE_NUM; i ++) {
            if(INVALIDE_SOCKET_FD != serial->fd[i])
                FD_SET(serial->fd[i], &rfds);
            else
                LOGATCI(LOG_DEBUG, "invalide fd set [%d].",i);
        }
///* for read rild data */

        //if the mode is dual talk, then we need to make sure which socket need to be connect by the property "ATCI_SIM"
        if(true == isRilAtciSocketExist) {

            if (isNeedSwitchSocket == true) {
                LOGATCI(LOG_DEBUG, "isNeedSwitchSocket is true, need reconnect to another modem. ");
                send(s_fdRild_command, "DISC", strlen("DISC"), 0);
                close(s_fdRild_command);
                connectTarget(RIL_TYPE);
                isNeedSwitchSocket = false;//after reconnect, reset it's value to false
            }

            if (s_fdRild_command < 0) {
                connectTarget(RIL_TYPE);
            }
            if (s_fdRild_command > 0) {
                FD_SET(s_fdRild_command, &rfds);
            } else {
                // if not usleep, connectTarget() will be run many times and too many log will be output.
                usleep(100*1000);
                return NULL;
            }
        }
        if (s_fdService_command > 0) {
            FD_SET(s_fdService_command, &rfds);
        }
        if (s_fdService_command_vendor > 0) {
            FD_SET(s_fdService_command_vendor, &rfds);
        }
        LOGATCI(LOG_DEBUG, "Wait for select data from USB");
        int maxFd = 0;
        for (i = 0; i < MAX_DEVICE_NUM; i++) {
            maxFd = MAX(maxFd, serial->fd[i]);
        }
        if(IS_RILDFDVALID()) {
            maxFd = MAX(s_fdRild_command, maxFd);
        }
        if(s_fdService_command > 0) {
            maxFd = MAX(s_fdService_command, maxFd);
        }
        if (s_fdService_command_vendor > 0) {
            maxFd = MAX(s_fdService_command_vendor, maxFd);
        }
        ret = select(maxFd + 1, &rfds, NULL, NULL, NULL);
        if (ret == -1) {
            if(errno == EINTR || errno == EAGAIN) {
                continue;
            }
            LOGATCI(LOG_ERR, "Fail to select in readline. error: %d", errno);
            for(i = 0; i < MAX_DEVICE_NUM; i ++) {
                close(serial->fd[i]);
                serial->fd[i] = INVALIDE_SOCKET_FD;
            }
///* for read rild data */
            close(s_fdRild_command);
            s_fdRild_command = INVALIDE_SOCKET_FD;
////
            return NULL;
        } else if(ret == 0) {
            LOGATCI(LOG_DEBUG, "ERROR:No data from USB devices");
            continue;
        }
///* for read rild data */
        if(IS_RILDFDVALID() && FD_ISSET(s_fdRild_command, &rfds)) {
            readDataFromTarget(RIL_TYPE, &s_fdRild_command);
            continue;
        }
////
        if(s_fdService_command > 0 && FD_ISSET(s_fdService_command, &rfds)) {
            LOGATCI(LOG_DEBUG, "readline: Prepare read the data from generic service");
            readDataFromTarget(GENERIC_TYPE, &s_fdService_command);
            continue;
        }
        if (s_fdService_command_vendor > 0 && FD_ISSET(s_fdService_command_vendor, &rfds)) {
            LOGATCI(LOG_DEBUG, "readline: Prepare read the data from generic service");
            readDataFromTarget(GENERIC_TYPE, &s_fdService_command_vendor);
            continue;
        }
        for(i = 0; i < MAX_DEVICE_NUM; i++) {
            if (serial->fd[i] != INVALIDE_SOCKET_FD && FD_ISSET(serial->fd[i], &rfds)) {
                LOGATCI(LOG_DEBUG, "Read data from USB:%d:%d", i, serial->fd[i]);
                fd = serial->fd[i];
                currDevice = i;
                if(ADB_SKT_SERVER_NUM == currDevice) {
                    serial->fd[ADB_SKT_CLIENT_NUM] = adb_socket_accept(fd);
                    atci_conn_mode = CONN_ADB;
                    p_buffer = tmpBuffer;
                } else if(ADB_SKT_CLIENT_NUM == currDevice) {
                    atci_conn_mode = CONN_ADB;
                    memset(tmpBuffer, 0, sizeof(tmpBuffer));
                    p_buffer = tmpBuffer;
                } else if (HIDL_CONN_NUM == currDevice) {
                    atci_conn_mode = CONN_HIDL;
                    memset(tmpBuffer, 0, sizeof(tmpBuffer));
                    p_buffer = tmpBuffer;
                } else {
                    atci_conn_mode = CONN_VCOM;
                    memset(buffer, 0, sizeof(buffer));
                    p_buffer = buffer;
                }
                break;
            }
        }

        do {
            if(currDevice == ADB_SKT_SERVER_NUM) {
                currDevice = ADB_SKT_CLIENT_NUM;
                fd = serial->fd[ADB_SKT_CLIENT_NUM];
            }

            LOGATCI(LOG_DEBUG, "Wait for read data from USB :%d", fd);
            readCount = read(fd, p_buffer, MAX_DATA_SIZE);  //Read data from VCOM driver

            if(readCount > 0) {
                LOGATCI(LOG_DEBUG, "the readCount is %d with %s:%d", readCount, p_buffer, p_buffer[0]);
                if(atci_conn_mode == CONN_VCOM) {
                    LOGATCI(LOG_DEBUG, "VCOM mode, sleep 0.01s");
                    usleep(10*1000);
                }
                break;
            } else if(readCount <= 0) {
                err = errno;
                if(err != EINTR && err != EAGAIN) {
                    LOGATCI(LOG_ERR, "FATAL Error in serail read:%d", err);
                    close(fd);

                    //Reopen this serial port
                    if(currDevice < MAX_DEVICE_VCOM_NUM) {
                        for(i = 0; i < MAX_FAILURE_RETRY; i++) {
                            serial->fd[currDevice] = open_serial_device(serial, serial->devicename[currDevice]);
                            if(serial->fd[currDevice] != INVALIDE_SOCKET_FD) {
                                LOGATCI(LOG_DEBUG, "The FD is reopen successfully");
                                return NULL;
                            }
                        }
                    } else {
                        if(currDevice == ADB_SKT_CLIENT_NUM)
                            serial->fd[ADB_SKT_CLIENT_NUM] = INVALIDE_SOCKET_FD;
                        LOGATCI(LOG_DEBUG, "currDevice[%d]",currDevice);
                        return NULL;
                    }
                    LOGATCI(LOG_ERR, "The process is terminated due to fatal error");
                    exit(1);
                }
                continue;
            }
        } while(1);

        // Update global status after successfully read, i.e. don't update when just handling error.
        // e.g. when machine is using socket but some error occcurs on COM,
        // we should handle the COM error but keep serial->currDevice be socket.
        serial->currDevice = currDevice;

        LOGATCI(LOG_DEBUG, "atci_conn_mode = [%d], [%x]",atci_conn_mode,p_buffer[0]);

        if (CONN_VCOM != atci_conn_mode && 0 < readCount) {
            int i = 0;
            for(i = 0; i < readCount; i++) {
                buffer[tmp_readCurr] = p_buffer[i];
                tmp_readCurr++;
                if ((AT_CR == p_buffer[i] || AT_LF == p_buffer[i])
                        || tmp_readCurr >= MAX_DATA_SIZE) {
                    readCount = tmp_readCurr;
                    tmp_readCurr = 0;
                    LOGATCI(LOG_DEBUG, ".AT command read done.");
                    break;
                } else {
                    if(i + 1 >= readCount) {
                        LOGATCI(LOG_DEBUG, ".AT command read more.");
                    }
                }
            }

            if(i >= readCount) {
                LOGATCI(LOG_DEBUG, ".go on read.");
                continue;
            }
        }
        LOGATCI(LOG_DEBUG, "state %d with %d:%d", serial->at_state, readCurr, buffer[readCurr]);

        if (isMsgCmdInput && !((int)buffer[0] == CTRLCHAR_LF && readCount == 1)) {
            if (isNeedMergeMsgCmdInput) {
                // merge this input after current parcial command
                processCtrlChar_BackSpace(buffer);
                LOGATCI(LOG_DEBUG, "isNeedMergeMsgCmdInput, text=%s", buffer);
                if (strlen(serial->ATBuffer) + strlen(buffer) + strlen("\"\r\n") + 1
                        <= (int) sizeof(serial->ATBuffer)) {
                    strncpy(serial->ATBuffer + strlen(serial->ATBuffer), buffer, strlen(buffer));
                    serial->totalSize += readCount;
                }
                LOGATCI(LOG_DEBUG, "isNeedMergeMsgCmdInput update buffer: %s", serial->ATBuffer);

                if (check_SUB_ESC_char(buffer)) {
                    // finish the command string
                    LOGATCI(LOG_DEBUG, " SUB or ESC in line, clear isNeedMergeMsgCmdInput");
                    isNeedMergeMsgCmdInput = false;
                    // -1 to backspace the ESC char
                    strncpy(serial->ATBuffer + strlen(serial->ATBuffer) - 1, "\"\r\n", strlen("\"\r\n"));
                    serial->totalSize += strlen("\"\r\n");
                    serial->ATBuffer[strlen("AT+")] = 'E';
                    LOGATCI(LOG_DEBUG, "isNeedMergeMsgCmdInput update buffer: %s", serial->ATBuffer);
                }
                break;
            } else {
                serial->totalSize = readCount;
                LOGATCI(LOG_DEBUG, "isMsgCmdInput, text=%s", buffer);
                if (sizeof(buffer) <= sizeof(serial->ATBuffer))
                    memcpy(serial->ATBuffer, buffer, sizeof(buffer));
                break;
            }
        } else {
            readCurr = audio_command_hdlr(buffer);
            if(readCurr <= 0) {
                readCurr = 0;
                headerSize = 2;
                processCtrlChar_BackSpace(buffer);
                if(serial->at_state == FIND_A && readCount > 2) {
                    if((UCASE(buffer[0]) == AT_A) && (UCASE(buffer[1]) == AT_T)) {
                        p_read = &buffer[0];
                        serial->at_state = FIND_DONE;
                        readCurr = 2;
                    }
                }
            } else {
                serial->at_state = FIND_DONE;
                headerSize = readCurr;
                p_read = &buffer[0];
            }

            //There is no AT string in this input; discard this input and get the input again.
            if(serial->at_state == FIND_DONE) {
                serial->totalSize = readCount - readCurr + headerSize; //Add the length of "AT"
                LOGATCI(LOG_DEBUG, "the readCurr is %d & the total size is %d", readCurr, serial->totalSize);
                memcpy(serial->ATBuffer, p_read, serial->totalSize);
                serial->ATBuffer[serial->totalSize-1] = '\0';  //Skip the "\r" character
                break;
            }
            memset(buffer, 0, sizeof(buffer));
        }
    }

    return &serial->ATBuffer[0];
}

int mmiCmdSeqEnQueue(int idx) {
    LOGATCI(LOG_DEBUG,"mmiCmdSeqEnQueue, idx[%d],seqIdx[%d],seq[%d]",idx,atRilMmi_info.endChkSeqIdx,atRilMmi_info.mmiCmdSeqQ[atRilMmi_info.endChkSeqIdx]);
    if(-1 == atRilMmi_info.mmiCmdSeqQ[atRilMmi_info.endChkSeqIdx])
        atRilMmi_info.mmiCmdSeqQ[atRilMmi_info.endChkSeqIdx] = idx;
    else
        return -1;

    atRilMmi_info.endChkSeqIdx++;

    if(MAX_QUEUE_MMI_CMD_IDX <= atRilMmi_info.endChkSeqIdx)
        atRilMmi_info.endChkSeqIdx = 0;
    return 0;
}

int mmiCmdSeqDeQueue() {
    int sIdx = atRilMmi_info.startChkSeqIdx;
    int ret = 0,cnt = 0;
    LOGATCI(LOG_DEBUG,"mmiCmdSeqDeQueue, seqIdx[%d],seq[%d]",sIdx,atRilMmi_info.mmiCmdSeqQ[sIdx]);
    while(MAX_QUEUE_MMI_CMD_IDX > cnt && 0 < atRilMmi_info.mmiCmdSeqQ[sIdx]) {
        LOGATCI(LOG_DEBUG,"seqIdx[%d],seq[%d]",sIdx,atRilMmi_info.mmiCmdSeqQ[sIdx]);
        atRilMmi_info.mmiCmdSeqQ[sIdx]--;
        if(0 == atRilMmi_info.mmiCmdSeqQ[sIdx]) {
            atRilMmi_info.mmiCmdSeqQ[sIdx] = -1;
            ret = 1; // need to reply to response to ATCIJ
            atRilMmi_info.startChkSeqIdx++;
            if(MAX_QUEUE_MMI_CMD_IDX <= atRilMmi_info.startChkSeqIdx)
                atRilMmi_info.startChkSeqIdx = 0;
        }
        sIdx++;
        cnt++;
        if(MAX_QUEUE_MMI_CMD_IDX == sIdx)
            sIdx = 0;
    }
    return ret;
}

void *readerLoop(void *arg) {
    Serial *p_serial = (Serial *)arg;
    ATCI_DataType dataType = UNKNOWN_TYPE;
    char operator[MTK_PROPERTY_VALUE_MAX] = {0};

    signal(SIGPIPE, sigpipe_handler);

    LOGATCI(LOG_DEBUG,"Enter");

    mtk_property_get(ATCI_IN_OPERATOR, operator, "OM");

    if (strcmp(operator, "OP07") == 0) {
        LOGATCI(LOG_INFO, "operator is OP07 and set PCT mode 0 for AT&T");
        mtk_property_set(ATCI_IN_PCT_PROP, "0");
    } else {
        mtk_property_set(ATCI_IN_PCT_PROP, "1");
    }

    connectTarget(GENERIC_TYPE);

    int fd[2] = {0};
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) >= 0) {
        serial.fd[HIDL_CONN_NUM] = fd[0];
        initAtcidHidlService(fd[1]);
    } else {
        LOGATCI(LOG_ERR, "socketpair failed. errno:%d", errno);
    }

    for (;;) {
        char* line = NULL;

        line = (char*) readline(p_serial);

        if (line == NULL) {
            continue;
        }

        if (isMsgCmdInput) {
            if (isNeedMergeMsgCmdInput) {
                writeDataToSerial(INTERMIDIATE_RSP, strlen(INTERMIDIATE_RSP) + 1);
            } else {
                if (true == isRilAtciSocketExist) {
                    if (s_fdRild_command < 0) {
                        connectTarget(RIL_TYPE);
                    }
                    sendDataToRild(line);
                }
            }
            continue;
        }

        convertToUpperCase(line,'=');
        trim_string(&line);
        LOGATCI(LOG_INFO, "Command:%s",line);

        dataType = process_cmd_line(line);

        LOGATCI(LOG_INFO, "The command type is belonged to :%d",dataType);

        if(dataType == RIL_TYPE) {
            if (isRilAtciSocketExist) {
                if (s_fdRild_command < 0) {
                    connectTarget(RIL_TYPE);
                }

                atRilMmi_info.cmdSeq++;  // Count we have send one command to RILD
                if (1 == atRilMmi_info.hasRilMMICmd) {
                    if (-1 == mmiCmdSeqEnQueue(atRilMmi_info.cmdSeq))
                        LOGATCI(LOG_ERR, "MMI cmd seq queue is full.");
                    atRilMmi_info.hasRilMMICmd = 0;
                }

                if (isMsgCmd(line)) {
                    LOGATCI(LOG_INFO, "set isMsgCmd true");
                    isMsgCmdInput = true;
                    if (strcmp(operator, "OP12") == 0
                            && startMergeMsgCmdInputIfNeed(p_serial, line)) {
                        isNeedMergeMsgCmdInput = true;
                        continue;
                    }
                }

                if (strncmp(line, "ATV1", 4) == 0) {
                    LOGATCI(LOG_INFO, "set Numeric flag false");
                    isNumericFormat = false;
                }
                if (strncmp(line, "ATV0", 4) == 0) {
                    LOGATCI(LOG_INFO, "set Numeric flag true");
                    isNumericFormat = true;
                }

                sendDataToRild(line);
            } else {
                if (s_fdRild_command < 0) {
                    LOGATCI(LOG_INFO, "Re-connect to RILD.");
                    isRilAtciSocketExist = true;
                    connectTarget(RIL_TYPE);
                    isNeedSwitchSocket = false;
                }

                atRilMmi_info.cmdSeq++;  // Count we have send one command to RILD
                if (1 == atRilMmi_info.hasRilMMICmd) {
                    if (-1 == mmiCmdSeqEnQueue(atRilMmi_info.cmdSeq))
                        LOGATCI(LOG_ERR, "MMI cmd seq queue is full.");
                    atRilMmi_info.hasRilMMICmd = 0;
                }

                if (isMsgCmd(line)) {
                    LOGATCI(LOG_INFO, "set isMsgCmd true");
                    isMsgCmdInput = true;
                    if (strcmp(operator, "OP12") == 0
                            && startMergeMsgCmdInputIfNeed(p_serial, line)) {
                        isNeedMergeMsgCmdInput = true;
                        continue;
                    }
                }

                if (strncmp(line, "ATV1", 4) == 0) {
                    LOGATCI(LOG_INFO, "set Numeric flag false");
                    isNumericFormat = false;
                }
                if (strncmp(line, "ATV0", 4) == 0) {
                    LOGATCI(LOG_INFO, "set Numeric flag true");
                    isNumericFormat = true;
                }

                if (IS_RILDFDVALID()) {
                    sendDataToRild(line);
                } else {
                    LOGATCI(LOG_INFO, "Response No Modem.2");
                    writeDataToSerial(NOMODEM_RSP, strlen(NOMODEM_RSP)+1);
                }
            }

#ifdef ATM_PCSENDAT_SUPPORT
            if (strcmp(line, META_CFG_CMD) == 0) {
                rildReaderLoop();
                if (isLastResponseOk) {
                    suspendAtciForMeta();
                }
            }
#endif
        } else if(dataType == ATCI_TYPE) {
            LOGATCI(LOG_INFO, "ATCI COMMAND");
        } else if(dataType == GENERIC_TYPE) {
            if (s_fdService_command < 0 || s_fdService_command_vendor < 0) {
                connectTarget(GENERIC_TYPE);
            }
            sendDataToGenericService(line);
            if(strcmp(line, "AT+POWERKEY") == 0) {
                LOGATCI(LOG_INFO, "+POWERKEY COMMAND");
                serviceReaderLoop();
            } else {
                LOGATCI(LOG_INFO, "Generic AT COMMAND");
            }
        } else if (dataType == AUDIO_TYPE) {
            LOGATCI(LOG_INFO, "AUDIO COMMAND");
            if (dataType == AUDIO_TYPE && s_fdAudio_command < 0) {
                connectTarget(dataType);
            }
            sendDataToAudio(line, p_serial->totalSize, dataType);
            audioReaderLoop(dataType);
        }
    }

    LOGATCI(LOG_INFO, "ReaderLoop thread Closed");

    return NULL;
}

/*
* Purpose:  Initial the default value of serial device.
* Input:      serial - the serial struct
* Return:    void
*/

void initSerialDevice(Serial *serial) {

    int i = 0;
    LOGATCI(LOG_DEBUG,"Enter");
    memset(serial,0,sizeof(Serial));
    memset(&atRilMmi_info,0,sizeof(At_Ril_Mmi_t));
    for(i = 0; i < MAX_QUEUE_MMI_CMD_IDX; i++)
        atRilMmi_info.mmiCmdSeqQ[i] = -1;

    for (i = 0; i < MAX_DEVICE_NUM; i++) {
        serial->fd[i] = INVALIDE_SOCKET_FD;
    }

    serial->echo[0] = 1;
    serial->echo[1] = 1;
    serial->totalSize = 0;
}

void setEchoOption(int flag) {
    struct termios deviceOptions;
    int fd = serial.fd[serial.currDevice];

    LOGATCI(LOG_INFO, "set echo option:%d for device:%d", flag, serial.currDevice);

    // get the parameters
    tcgetattr(fd,&deviceOptions);
    if(flag == 1) {
        deviceOptions.c_lflag = ICANON | ECHO;
    } else {
        deviceOptions.c_lflag = ICANON ;
    }

    serial.echo[serial.currDevice] = flag;

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW,&deviceOptions);

}

/*
* Purpose:  Open and initialize the serial device used.
* Input:      serial - the serial struct
* Return:    0 if port successfully opened, else -1.
*/
int open_serial_device(Serial* serial, char* devicename) {
    LOGATCI(LOG_DEBUG, "Enter");
    int fdflags;
    struct termios deviceOptions;
    int fd = INVALIDE_SOCKET_FD;
    int is_echo = 1;
    int ret = 0;

    LOGATCI(LOG_INFO, "Opened serial port:%s", devicename);

    do {
        fd = open(devicename, O_RDWR | O_NOCTTY | O_NONBLOCK);
    } while (fd < 0 && errno == EINTR);

    if(fd < 0) {
        LOGATCI(LOG_ERR, "Fail to open serial device(%s) with error code:%d", devicename, errno);
        return INVALIDE_SOCKET_FD;
    }

    if(strcmp(devicename, TTY_GS0) == 0) {
        is_echo = serial->echo[0];
    }
#ifdef MTK_TC1_FEATURE
    else if(strcmp(devicename, TTY_GS4) == 0) {
        is_echo = serial->echo[1];
    }
#else
    else if(strcmp(devicename, TTY_GS1) == 0) {
        is_echo = serial->echo[1];
    }
#endif

    LOGATCI(LOG_DEBUG, "serial->fd is %d ", fd);

    // Set FD to Sync IO
    fdflags = fcntl(fd, F_GETFL);
    ret = fcntl(fd, F_SETFL, fdflags & ~O_NONBLOCK);
    if (ret < 0) {
        LOGATCI(LOG_ERR, "Error clearing O_NONBLOCK errno: %d", errno);
    }

    // get the parameters
    tcgetattr(fd,&deviceOptions);

    // set raw input
    if(is_echo == 1) {
        deviceOptions.c_lflag = ICANON | ECHO;
    } else {
        deviceOptions.c_lflag = ICANON ;
    }

    // set raw output
    deviceOptions.c_oflag &= ~OPOST;
    deviceOptions.c_oflag &= ~OLCUC;
    deviceOptions.c_oflag &= ~ONLRET;
    deviceOptions.c_oflag &= ~ONOCR;
    deviceOptions.c_oflag &= ~OCRNL;

    deviceOptions.c_cc[VEOL] = 0x1a; /*Ctrl+Z*/
    deviceOptions.c_lflag = deviceOptions.c_lflag | IEXTEN;
    deviceOptions.c_cc[VEOL2] = 0x1b; /*ESC*/

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW,&deviceOptions);

    return fd;
}

void setSocketConnect(SocketId socketId) {
    if ((connc_socket != socketId)) { //need switch socket connect from md2 into md1
        LOGATCI(LOG_DEBUG, "Set to new socketId");
        connc_socket = socketId;
        isNeedSwitchSocket = true;
    } else {
        LOGATCI(LOG_DEBUG, "no need to change socket");
        isNeedSwitchSocket = false;
    }
}

SocketId getSocketConnect(){
    return connc_socket;
}


int adb_socket_accept(int socketFd) {
    struct sockaddr_in clientAddr;
    socklen_t sinSize = sizeof(struct sockaddr_in);
    int fd_clientSk = INVALIDE_SOCKET_FD;

    LOGATCI(LOG_DEBUG, "accept socket on fd :%d",socketFd);

    if(-1 == (fd_clientSk = accept(socketFd, (struct sockaddr *)&clientAddr, &sinSize))) {
        LOGATCI(LOG_DEBUG, "Fail to accept socket port err:%d",errno);
    }
    return fd_clientSk;
}
int adb_socket_listen(int socketFd) {
    LOGATCI(LOG_DEBUG, "Listen socket on fd :%d",socketFd);
    if(-1 == listen(socketFd,BACK_LOG)) {
        LOGATCI(LOG_DEBUG, "Fail to listen socket port err:%d",errno);
        return ATCID_CREATE_ADB_SOCKET_ERR;
    }
    return ATCID_SUCCESS;
}

/** returns 1 if line starts with prefix, 0 if it does not */
int strStartsWith(const char *line, const char *prefix) {
    for ( ; *line != '\0' && *prefix != '\0' ; line++, prefix++) {
        if (*line != *prefix) {
            return 0;
        }
    }

    return *prefix == '\0';
}

char* cut_msg_line(char* line) {
    char* result;
    int i = 0;
    int size = strlen(line);
    int len = size;
    for(i = 0; i < size; i++) {
        if(line[i] == '=' && line[i+1] != '?' && line[i+1] != '\0' ) {
            len = i;
            break;
        }
    }
    result = (char*)malloc((len + 1) * sizeof(char));
    if (result == NULL) {
        LOGATCI(LOG_DEBUG, "malloc result fail, return NULL");
        return NULL;
    }
    memcpy(result, line, len);
    result[len] = '\0';
    return result;
}

int isMsgCmd(char* line) {
    int i = 0;
    int table_size = 0;
    char* line_cut = cut_msg_line(line);
    int result = -1;
    int err = -1;
    char* tmp = NULL;
    char* tmpBuf = NULL;

    if (line_cut == NULL) {
        return 0;
    }
    table_size = (sizeof(msg_cmd_table)/sizeof(generic_cmd_type));

    for(i = 0; i < table_size; i++) {
        if(strcmp(line_cut, msg_cmd_table[i].cmdName) == 0) {
            free(line_cut);
            return 1;
        }
    }

    LOGATCI(LOG_DEBUG, "line_cut: %s", line_cut);
    if (strcmp(line_cut, "AT+CMGF") == 0) {
        tmpBuf = (char *)malloc(strlen(line) + 1);
        memcpy(tmpBuf, line, strlen(line) + 1);

        tmp = tmpBuf;
        err = at_tok_start_flag(&tmp, '=');
        if (err == 0 && at_tok_hasmore(&tmp) && tmp[0] != '?') {
            err = at_tok_nextint(&tmp, &result);
        }
        if (err == 0) {
            msgCmdMode = result;
            LOGATCI(LOG_DEBUG, "msgCmdMode = %d", result);
        }

        free(tmpBuf);
    }

    free(line_cut);
    return 0;
}

int isMsgResponse(const char *line) {
    size_t i;
    for (i = 0; i < NUM_ELEMS(s_MsgResponses); i++) {
        if (strStartsWith(line, s_MsgResponses[i])) {
            return 1;
        }
    }
    return 0;
}

int getMsgCmdParameterCount(char* line) {
    int err = -1;
    char* out = NULL;
    char* tmp = NULL;
    char* tmpBuf = NULL;
    int count = 0;

    err = at_tok_start_flag(&line, '=');
    if (err < 0 || line[0] == '?') {
        return 0;
    }

    tmpBuf = (char *)malloc(strlen(line) + 1);
    if (tmpBuf == NULL) {
        LOGATCI(LOG_DEBUG, "fail to malloc tmpBuf, return 0");
        return 0;
    }

    memcpy(tmpBuf, line, strlen(line) + 1);

    tmp = tmpBuf;
    while (at_tok_hasmore(&tmp)) {
        err = at_tok_nextstr(&tmp, &out);
        if (err == 0) {
            count++;
        }
    }
    free(tmpBuf);

    return count;
}

int startMergeMsgCmdInputIfNeed(Serial *serial, char* line) {
    // for msg cmd, merge multi line input to single command if need
    char simIDProperty[MTK_PROPERTY_VALUE_MAX];
    int simID = 0;
    mtk_property_get(ATCI_SIM, simIDProperty, "0");
    simID = atoi(simIDProperty);
    if (simID == 9) {
        return 0;
    }

    int count = getMsgCmdParameterCount(line);
    LOGATCI(LOG_DEBUG, "startMergeMsgCmdInputIfNeed count %d msgCmdMode %d", count, msgCmdMode);
    if (count == 1 || (count == 2 && msgCmdMode == 1)) {
        const char* str = ",\"";
        if (count == 1 && msgCmdMode == 1) {
            str = ",,\"";
        }
        strncpy(serial->ATBuffer + strlen(serial->ATBuffer), str, strlen(str));
        serial->totalSize += strlen(str);
        LOGATCI(LOG_DEBUG, "startMergeMsgCmdInputIfNeed update buffer: %s", serial->ATBuffer);
        writeDataToSerial(INTERMIDIATE_RSP, strlen(INTERMIDIATE_RSP) + 1);
        return 1;
    }
    return 0;
}

int isDTSupport() {
    int result = 0;

#ifdef MTK_DT_SUPPORT
    result = 1;
#endif

    return result;
}

bool Response2Numeric(char *input, char *output) {
    if (isNumericFormat) {
        size_t i;
        for (i = 0; i < NUM_ELEMS(numericResponse_table); i++) {
            if (strcmp(input, numericResponse_table[i].Response) == 0) {
                LOGATCI(LOG_DEBUG, "Response2Numeric, i=%d", (int) i);
                memcpy(output, numericResponse_table[i].Number, 1);
                return true;
            }
        }
        memcpy(output, input, strlen(input));
    }
    return false;
}

void readDataFromTargetSync(ATCI_DataType dataType, int* s_fd_listen, char* response) {
    int recvLen = 0;
    char buffer[MAX_DATA_SIZE + 1];

    LOGATCI(LOG_DEBUG, "Enter");
    LOGATCI(LOG_INFO, "Read data from target:%d with fd:%d", dataType, *s_fd_listen);

    memset(buffer, 0, sizeof(buffer));

    do {
        recvLen = recv(*s_fd_listen, buffer, MAX_DATA_SIZE, 0);
        if (recvLen == -1) {
            if (errno == EAGAIN || errno == EINTR) {
                continue;
            }
            LOGATCI(LOG_ERR, "fail to receive data from target socket. errno = %d", errno);
            close(*s_fd_listen);
            *s_fd_listen = INVALIDE_SOCKET_FD;
            return;
        } else if (recvLen == 0) {
            LOGATCI(LOG_ERR, "The peer has performed an orderly shutdown, isATCmdInject %d", isATCmdInject);
            close(*s_fd_listen);
            *s_fd_listen = INVALIDE_SOCKET_FD;
            return;
        }
    } while (recvLen <= 0);

    LOGATCI(LOG_INFO, "data receive from %d is %s", dataType, buffer);
    LOGATCI(LOG_INFO, "data receive data length is %d", recvLen);
    strncpy(response, buffer, MAX_DATA_SIZE);
}

void rildReaderLoopSync(char* response) {
    int ret;
    fd_set rfds;

    LOGATCI(LOG_DEBUG, "Enter");

    if (s_fdRild_command != INVALIDE_SOCKET_FD) {
        FD_ZERO(&rfds);
        FD_SET(s_fdRild_command, &rfds);

        ret = select(s_fdRild_command+1, &rfds, NULL, NULL, NULL);
        if (ret < 0) {
            if (errno == EINTR || errno == EAGAIN) {
                return;
            }
            LOGATCI(LOG_ERR, "Fail to select in rildReaderLoop. error: %d", errno);
            close(s_fdRild_command);
            s_fdRild_command = INVALIDE_SOCKET_FD;
            return;
        } else if (ret == 0) {
            LOGATCI(LOG_DEBUG, "timeout for select in rildREaderLoop");
            close(s_fdRild_command);
            s_fdRild_command = INVALIDE_SOCKET_FD;
            return;
        }

        if (FD_ISSET(s_fdRild_command, &rfds)) {
            LOGATCI(LOG_DEBUG, "Prepare read the data from rild driver");
            readDataFromTargetSync(RIL_TYPE, &s_fdRild_command, response);
        }
    }
}

int sendDataToRildSync(char* line, char* response) {
    int len = 0;
    LOGATCI(LOG_DEBUG, "Enter");

    len = strlen(line);
    if (sendDataToRild(line) < 0) {
        snprintf(response, MAX_DATA_SIZE, NOMODEM_RSP);
    }
    LOGATCI(LOG_DEBUG, "send data over");

    rildReaderLoopSync(response);
    return 0;
}

