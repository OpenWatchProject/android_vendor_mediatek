/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#include <utils/Log.h>
#include <cutils/properties.h>
#include <string.h>
#include <iostream>
#include <thread>
#include <cutils/sockets.h>
#include <limits.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include "ImsaClient.h"
/*****************************************************************************
 *
 *****************************************************************************/
#define ARRAY_LENGTH(array) (sizeof(array)/sizeof(array[0]))

using namespace std;
using namespace android;

#define IMSA_MAX_COUNT 10
#define UNUSED(x) (x)   //eliminate "warning: unused parameter"
#define IMSM_SOCKET_NAME (char *)"/dev/socket/volte_imsm"

static pthread_mutex_t s_pendingEventsMutex = PTHREAD_MUTEX_INITIALIZER;
static EventInfo *s_pendingEvents = NULL;

extern "C" void eventNotify(int8_t* buffer, int32_t request_id, int32_t length);

ImsaClient::ImsaClient() {
    ALOGD("ImsaClient init done");
}

ImsaClient::~ImsaClient() {

}

void ImsaClient::writeEvent(int32_t request_id, int32_t length, char* data) {
    EventInfo *event = (EventInfo *)calloc(1, sizeof(EventInfo));
    event->requestId = request_id;
    event->dataLength = length;
    if (length > 0) {
        event->dataBuffer = calloc(length, sizeof(char));
        memcpy(event->dataBuffer, data, length);
    } else {
        event->dataBuffer = NULL;
    }
    event->p_next = NULL;
    addEventRequest(event);
    // Thread for send IMSM message
    pthread_attr_t  attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    int ret = pthread_create(&g_imsa_send_thread, &(attr), IMSA_Send_Thread, NULL);
    pthread_attr_destroy(&attr);
    if (ret < 0) {
        ALOGD("IMSA : Fail to create thread");
    }
}

extern "C"
void addEventRequest(EventInfo *event) {
    if (event == NULL) {
        ALOGD("IMSA : addEventRequest, event is NULL");
        return;
    }
    pthread_mutex_t* pendingRequestsEventsMutexHook = &s_pendingEventsMutex;
    pthread_mutex_lock(pendingRequestsEventsMutexHook);

    if (s_pendingEvents == NULL) {
        s_pendingEvents = event;
        ALOGD("IMSA : add first event, event = %p, requestid = %d", event, event->requestId);
    } else {
        for(EventInfo *pCur = s_pendingEvents
            ; pCur != NULL
            ; pCur = pCur->p_next) {
            if (pCur != NULL && pCur->p_next == NULL) {
                pCur->p_next = event;
                ALOGD("IMSA : add event on p_next, event = %p, requestid = %d", event, event->requestId);
                break;
            }
        }
    }

    pthread_mutex_unlock(pendingRequestsEventsMutexHook);
}

extern "C"
int connectToSocket(char *socketName) {
    int socketFd;
    const int one = 1;
    struct sockaddr_un imsm_addr;
    int res = 0;

    if (g_imsa_imsm_fd > 0) {
        close(g_imsa_imsm_fd); //close old fd first to prevent fd leak
    }

    socketFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socketFd < 0) {
        ALOGD("IMSA : create socket failed");
        return -1;
    }

    res = setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if (res < 0) {
       ALOGD("IMSA : setsockopt failed");
       close(socketFd);
       return -1;
    }
    memset(&imsm_addr, 0, sizeof(imsm_addr));
    imsm_addr.sun_family = AF_UNIX;
    strlcpy(imsm_addr.sun_path, socketName,
        sizeof(imsm_addr.sun_path));
    if (TEMP_FAILURE_RETRY(connect(socketFd,
                 (const struct sockaddr*) &imsm_addr,
                 sizeof(imsm_addr))) != 0) {
        //ALOGD("IMSA : connect to %s failed, fd=%d, err: %d(%s)",
        //        socketName, socketFd, errno, strerror(errno));
        close(socketFd);
        return -1;
    }

    ALOGD("IMSA : connect socket success, fd=%d", socketFd);
    // Thread for send pending event message
    pthread_attr_t  attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    int ret = pthread_create(&g_imsa_pending_thread, &(attr), IMSA_Send_Thread, NULL);
    pthread_attr_destroy(&attr);
    if (ret < 0) {
        ALOGD("IMSA : Fail to create pending event thread");
    }

    return socketFd;
}

static void *IMSA_Recv_Thread(void *arg) {
    UNUSED(arg);
    g_imsa_imsm_fd = connectToSocket(IMSM_SOCKET_NAME);
    int retry = 0;
    while (g_imsa_imsm_fd < 0) {
        retry++;
        usleep(2000 * 1000);
        ALOGD("IMSA : des = /dev/socket/volte_imsm, Fail to connect . retry count: %d", retry);
        g_imsa_imsm_fd = connectToSocket(IMSM_SOCKET_NAME);
    }

    ALOGD("IMSA : IMSA_Recv_Thread start ");
    while (1) {
        int request_id;
        unsigned int recv_length = 0;
        int8_t* outBuffer = NULL;

        int ret = recv(g_imsa_imsm_fd, &request_id, sizeof(request_id), 0);
        if (ret != sizeof(request_id)) {
            //ALOGD("IMSA : recv fail (request_id) / ret = %d / fd: %d, try to re-connect",
                //ret, g_imsa_imsm_fd);
            g_imsa_imsm_fd = connectToSocket(IMSM_SOCKET_NAME);
            continue;
        }

        ret = recv(g_imsa_imsm_fd, &recv_length, sizeof(recv_length), 0);
        if (ret != sizeof(recv_length)) {
            //ALOGD("IMSA : recv fail (length) / ret = %d / fd: %d, try to re-connect",
                //ret, g_imsa_imsm_fd);
            g_imsa_imsm_fd = connectToSocket(IMSM_SOCKET_NAME);
            continue;
        }

        if((UINT_MAX / sizeof(int8_t)) <= recv_length){
            ALOGD("IMSA : recv_length overflow : %d vs %d", (sizeof(int8_t)*recv_length),INT_MAX);
            continue;
        }
        outBuffer = (int8_t* )malloc((sizeof(int8_t)*recv_length) + 1);
        if(outBuffer == NULL ){
            ALOGD("IMSA : buffer request fail as null");
            continue;
        }

        ret = recv(g_imsa_imsm_fd, reinterpret_cast<void *>(outBuffer), recv_length, 0);
        if (ret != recv_length) {
            //ALOGD("IMSA : recv fail (data) / ret = %d / fd: %d, try to re-connect",
                //ret, g_imsa_imsm_fd);
            g_imsa_imsm_fd = connectToSocket(IMSM_SOCKET_NAME);
            free(outBuffer);
            continue;
        }

        ALOGD("IMSA : request_id = %d, recv_length : %d, outBuffer = %p", request_id, recv_length, outBuffer);
        eventNotify(outBuffer, request_id, recv_length);
        free(outBuffer);
    }
    return 0;
}

static void *IMSA_Send_Thread(void *arg) {
    UNUSED(arg);
    if (g_imsa_imsm_fd == -1) {
        ALOGD("IMSA : g_imsa_imsm_fd = -1 when send !!");
        return 0;
    }

    ALOGD("IMSA : IMSA_Send_Thread start ");

    pthread_mutex_t* pendingRequestsEventsMutexHook = &s_pendingEventsMutex;
    pthread_mutex_lock(pendingRequestsEventsMutexHook);

    EventInfo* event_temp = NULL;

    while (s_pendingEvents != NULL) {
        ALOGD("IMSA : IMSA_Send_Thread : s_pendingEvents = %p, s_pendingEvents->requestId = %d, s_pendingEvents->dataLength = %d, s_pendingEvents->dataBuffer = %p,  ", 
            s_pendingEvents, s_pendingEvents->requestId, s_pendingEvents->dataLength, s_pendingEvents->dataBuffer);

        if (send(g_imsa_imsm_fd, (const void*)&s_pendingEvents->requestId, sizeof(int), 0) != sizeof(int)) {
            ALOGD("IMSA : send fail (requestId) / fd: %d, errCode: %d",
                    g_imsa_imsm_fd, errno);
            break;
        }
        if (s_pendingEvents->dataLength <= 0) {
            ALOGD("IMSA : event->dataLength <= 0");
        }
        if (send(g_imsa_imsm_fd, (const void*)&s_pendingEvents->dataLength, sizeof(int), 0) != sizeof(int)) {
            ALOGD("IMSA : send fail (dataLength) / fd: %d, errCode: %d",
                    g_imsa_imsm_fd, errno);
            break;
        }

        if (send(g_imsa_imsm_fd, (const void*)s_pendingEvents->dataBuffer, s_pendingEvents->dataLength, 0) != s_pendingEvents->dataLength) {
            ALOGD("IMSA : send fail (data) / fd: %d, errCode: %d",
                    g_imsa_imsm_fd, errno);
            break;
        }

        event_temp = s_pendingEvents;
        s_pendingEvents = s_pendingEvents->p_next;
        if (event_temp->dataBuffer != NULL) {
            free(event_temp->dataBuffer);
        }
        free(event_temp);
    }

    pthread_mutex_unlock(pendingRequestsEventsMutexHook);
    return 0;
}

extern "C"
int IMSA_Connect(void) {
    // Thread for recv IMSM message
    pthread_attr_t  attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    int ret = pthread_create(&g_imsa_recv_thread, &(attr), IMSA_Recv_Thread, NULL);
    pthread_attr_destroy(&attr);
    if (ret < 0) {
        ALOGD("IMSA : Fail to create recv thread");
        return 0;
    }
    return 1;
}
