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

#include <log/log.h>
#include <assert.h>
#include <pthread.h>
#include <cutils/properties.h>
#include <cutils/sockets.h>
#include <telephony/record_stream.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/select.h>

#include <telephony/mtk_ril.h>
#include "Rfx.h"
#include "socket_channel.h"
#include "socket_util.h"
#include "RfxMainThread.h"
#include "util/RpFeatureOptionUtils.h"

#undef LOG_TAG
#define LOG_TAG "RilSoktChanl"

extern "C"
void setRadioState(RIL_RadioState newState, RIL_SOCKET_ID rid);

extern "C"
void decodeAndSendSapMessage(void *data, size_t datalen,
        RIL_SOCKET_ID socketId);

#define MAX_COMMAND_BYTES (20 * 1024)
#define MAX_DATA_SIZE 2048

extern void *ril_socket_reader_looper(void *arg);
extern void *ril_atci_socket_reader_looper(void *arg);
extern void *sap_ril_proxy_rild_socket_reader_looper(void *arg);

void RilpSocket::connectSocket(void)
{
    RLOGI ("connectSocket to %d %d socket_name %s\n", group, id, name);
    while (1) {
        socketFd = socket_local_client(name,
                ANDROID_SOCKET_NAMESPACE_RESERVED,
                SOCK_STREAM);
        if(socketFd > 0) {
            rfx_set_socket(group, id, socketFd);
            return;
        } else {
            RLOGI("connectSocket fail, try again name:%s\n", name);
            sleep(1);
        }
    }
}

const char *RilpSocket::rildGroupIdtoString(RILD_RadioTechnology_Group group) {
    switch (group) {
        case RADIO_TECH_GROUP_GSM:
            return "GSM";
        case RADIO_TECH_GROUP_C2K:
            return "C2K";
        // External SIM [Start]
        case RADIO_TECH_GROUP_VSIM:
            return "VSIM";
        // External SIM [End]
        default:
            RLOGI ("Wrong group id: %d\n", group);
            break;
    }
    return NULL;
}

void RilpSocket::readerLooper(void) {
    RecordStream *p_rs = record_stream_new(socketFd, MAX_COMMAND_BYTES);
    void *p_record;
    size_t recordlen;
    int ret;

    RLOGI ("readerLooper Group: %s, SlotId: %d, socketFd:%d\n", rildGroupIdtoString(group), id, socketFd);

    while (1) {
        for (;;) {
            /* loop until EAGAIN/EINTR, end of stream, or other error */
            ret = record_stream_get_next(p_rs, &p_record, &recordlen);

            if (ret == 0 && p_record == NULL) {
                /* end-of-stream */
                break;
            } else if (ret < 0) {
                break;
            } else if (ret == 0) { /* && p_record != NULL */
                rfx_process_raw_data(group, id, p_record, recordlen);
            }
        }
        if (ret == 0 || !(errno == EAGAIN || errno == EINTR)) {
            /* fatal error or end-of-stream */
            if (ret != 0) {
                RLOGE("error on reading command socket errno:%d\n", errno);
            } else {
                RLOGW("EOS.  Closing command socket.");
            }

            close(socketFd);
            record_stream_free(p_rs);
            socketFd = -1;
            return;
        }
    }
}

void RilpSocket::initSocket(const char *socketName, RIL_SOCKET_ID socketid, RILD_RadioTechnology_Group groupId) {
    pthread_attr_t attr;
    PthreadPtr pptr = ril_socket_reader_looper;
    int result;

    RilpSocket* socket = new RilpSocket(socketName, socketid, groupId);
    socket->connectSocket();

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    //Start socket data receiving loop thread
    result = pthread_create(&socket->socketThreadId, &attr, pptr, socket);
    if(result < 0) {
        RLOGE("pthread_create failed with result:%d",result);
    }
}

void SapRilProxyRildSocket::initSocket(const char *socketName, RIL_SOCKET_ID socketid,
        RILD_RadioTechnology_Group groupId) {
    pthread_attr_t attr;
    PthreadPtr pptr = sap_ril_proxy_rild_socket_reader_looper;
    int result;

    SapRilProxyRildSocket* socket = new SapRilProxyRildSocket(socketName, socketid, groupId);
    socket->connectSocket();

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    // Start socket data receiving loop thread
    result = pthread_create(&socket->socketThreadId, &attr, pptr, socket);
    if (result < 0) {
        RLOGE("pthread_create failed with result:%d", result);
    }
}

void SapRilProxyRildSocket::connectSocket(void) {
    RLOGI("connectSocket to %d socket_name %s\n", id, name);
    int ret;

    while (1) {
        socketFd = socket_local_client(name,
                ANDROID_SOCKET_NAMESPACE_RESERVED,
                SOCK_STREAM);
        if(socketFd > 0) {
            rfx_set_sap_socket(id , group, socketFd);
            return;
        } else {
            RLOGI ("connectSocket fail, try again name:%s\n", name);
            sleep(1);
        }
    }
}

void SapRilProxyRildSocket::readerLooper(void) {
    RecordStream *p_rs = record_stream_new(socketFd, MAX_COMMAND_BYTES);
    void *p_record;
    size_t recordlen;
    int ret;

    RLOGI ("readerLooper Group: %s, SlotId: %d, socketFd:%d\n", rildGroupIdtoString(group), id,
            socketFd);

    while (1) {
        for (;;) {
            /* loop until EAGAIN/EINTR, end of stream, or other error */
            ret = record_stream_get_next(p_rs, &p_record, &recordlen);

            if (ret == 0 && p_record == NULL) {
                /* end-of-stream */
                break;
            } else if (ret < 0) {
                break;
            } else if (ret == 0) { /* && p_record != NULL */
                decodeAndSendSapMessage(p_record, recordlen, id);
            }
        }
        if (ret == 0 || !(errno == EAGAIN || errno == EINTR)) {
            /* fatal error or end-of-stream */
            if (ret != 0) {
                RLOGE("error on reading command socket errno:%d\n", errno);
            } else {
                RLOGW("EOS.  Closing command socket.");
            }

            close(socketFd);
            record_stream_free(p_rs);
            socketFd = -1;
            return;
        }
    }
}

const char* SapRilProxyRildSocket::rildGroupIdtoString(RILD_RadioTechnology_Group group) {
    switch (group) {
        case RADIO_TECH_GROUP_SAP_GSM:
            return "GSM SAP";
        case RADIO_TECH_GROUP_SAP_C2K:
            return "C2K SAP";
        default:
            RLOGI ("Wrong group id: %d\n", group);
            break;
    }
    return NULL;
}

extern "C"
void ril_socket_init() {
    int i=0;
    char *socketName;
    for(i=0; i<SIM_COUNT; i++) {
        socketName = RilpSocketUtil::getSocketName(RADIO_TECH_GROUP_GSM, RIL_SOCKET_ID(i));
        RilpSocket::initSocket(socketName, RIL_SOCKET_ID(i), RADIO_TECH_GROUP_GSM);
    }

    socketName = RilpSocketUtil::getSocketName(RADIO_TECH_GROUP_C2K, RIL_SOCKET_1);
    if(RpFeatureOptionUtils::isC2kSupport()) {
        RilpSocket::initSocket(socketName, RIL_SOCKET_1, RADIO_TECH_GROUP_C2K);
    }

    // External SIM [Start]
    for(i=0; i<SIM_COUNT; i++) {
        socketName = RilpSocketUtil::getSocketName(RADIO_TECH_GROUP_VSIM, RIL_SOCKET_ID(i));
        RilpSocket::initSocket(socketName, RIL_SOCKET_ID(i), RADIO_TECH_GROUP_VSIM);
    }
    // External SIM [End]

    for (i=0; i<SIM_COUNT; i++) {
        setRadioState(RADIO_STATE_OFF, RIL_SOCKET_ID(i));
    }

    // ril-proxy to rild socket
    for (i=0; i<SIM_COUNT; i++) {
        socketName = RilpSocketUtil::getSocketName(RADIO_TECH_GROUP_SAP_GSM, RIL_SOCKET_ID(i));
        SapRilProxyRildSocket::initSocket(socketName, RIL_SOCKET_ID(i), RADIO_TECH_GROUP_SAP_GSM);
        if (RpFeatureOptionUtils::isC2kSupport()) {
            socketName = RilpSocketUtil::getSocketName(RADIO_TECH_GROUP_SAP_C2K, RIL_SOCKET_ID(i));
            SapRilProxyRildSocket::initSocket(socketName, RIL_SOCKET_ID(i),
                    RADIO_TECH_GROUP_SAP_C2K);
        }
    }
}

extern "C"
void *ril_socket_reader_looper(void *arg) {
    RilpSocket *socket = (RilpSocket *)arg;
    while (1) {
        socket->readerLooper();
        rfx_close_socket(socket->group, socket->id);

        for(int i=0; i<SIM_COUNT; i++) {
            // always set to unavailable when socket is disconnected
            setRadioState(RADIO_STATE_UNAVAILABLE, RIL_SOCKET_ID(i));
        }

        RLOGI("reconnect socket");
        socket->connectSocket();
    }
    free(socket);
    RLOGI("end of ril_socket_reader_looper");
    return NULL;
}

extern "C"
void *sap_ril_proxy_rild_socket_reader_looper(void *arg) {
    SapRilProxyRildSocket *socket = (SapRilProxyRildSocket *)arg;
    while (1) {
        socket->readerLooper();

        rfx_set_sap_socket(socket->id , socket->group, -1);
        RLOGI("reconnect socket");
        socket->connectSocket();
    }
    free(socket);
    RLOGI("end of sap_ril_proxy_rild_socket_reader_looper");
    return NULL;
}
