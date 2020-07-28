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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#include "SocketListener.h"
#include "Mdmi_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <errno.h>

#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>/* for Unix domain sockets */

static const char *TAG = "MDMI-SocketListener";

SocketListener::SocketListener(const char *szServerName):
        m_maxFD(-1), m_threadId(pthread_self()),
        m_notifyListenThreadStop(false), m_connHandlerInstance(NULL) {
    FD_ZERO(&m_masterFDs);
    if (!SetupDomainSocket(szServerName)) {
        MDMI_LOGE(TAG, "Fail to setup domain socket.");
        Cleanup();
        return;
    }
    if (pthread_create(&m_threadId, NULL, SocketListener::ListenThread, this)) {
        MDMI_LOGE(TAG, "Fail to create MonitorCmdService's listener thread, error=%s", strerror(errno));
        Cleanup();
    }
    if (0 != pthread_setname_np(m_threadId, "mdm_cmd_serv")) {
        MDMI_LOGE(TAG, "Fail to set name for MonitorCmdService's listener thread, error=%s",
                strerror(errno));
    }
}

SocketListener::~SocketListener() {
    if (0 != pthread_equal(m_threadId, pthread_self())) {
        /* Equal, thread is cancelled. */
        MDMI_LOGD(TAG, "Thread is cancelled.");
    } else {
        m_notifyListenThreadStop = true;
        MDMI_LOGD(TAG, "Stopping Listen thread ....");
        if (0 != pthread_join(m_threadId, NULL)) {
            MDMI_LOGE(TAG, "pthread_join failed. err = [%s](%d)", strerror(errno), errno);
        }
        m_threadId = pthread_self();
    }
    Cleanup();
}

void SocketListener::SetConnectionHandler(ConnectionHandlerInterface *handlerInstance) {
    m_connHandlerInstance = handlerInstance;
}

bool SocketListener::AddSocket(SOCKET_TYPE type, int fd, const char *szAddr) {
    MDMI_LOGD(TAG, "AddSocket fd = [%d]", fd);
    std::map<int, SocketConnection *>::iterator itr;
    if ((itr = m_socket[type].find(fd)) != m_socket[type].end()) {
        itr->second->SetAddr(szAddr);
    } else {
        SocketConnection *pConn = NULL;
        if (NULL == (pConn = (SocketConnection *)new SocketConnection(fd, szAddr))) {
            return false;
        }
        m_socket[type].insert(std::make_pair(fd, pConn));
        FD_SET(fd, &m_masterFDs);
        if (fd > m_maxFD) {
            m_maxFD = fd;
        }
    }
    return true;
}

void SocketListener::RemoveSocket(SOCKET_TYPE type, int fd) {
    MDMI_LOGD(TAG, "RemoveSocket fd = [%d]", fd);
    std::map<int, SocketConnection *>::iterator itr;
    if ((itr = m_socket[type].find(fd)) != m_socket[type].end()) {
        delete itr->second;
        m_socket[type].erase(itr);
    }
    FD_CLR(fd, &m_masterFDs);
}

void SocketListener::Cleanup() {
    for (int i = 0; i < SOCKET_TYPE_SIZE; ++i) {
        std::map<int, SocketConnection *>::iterator socketItr;
        for (socketItr = m_socket[i].begin(); socketItr != m_socket[i].end(); ++socketItr) {
            delete socketItr->second;
        }
        m_socket[i].clear();
    }
}

bool SocketListener::SetupDomainSocket(const char *szServerName) {
    int fd = -1;
    struct sockaddr_un servaddr; /* Structs for the client and server socket addresses. */
    size_t serverNameLen = 0;

    if (NULL == szServerName) {
        MDMI_LOGE(TAG, "szServerName is NULL");
        return false;
    }

    MDMI_LOGD(TAG, "SetupDomainSocket [%s]", szServerName);

    if (0 > (fd = socket( AF_LOCAL, SOCK_STREAM | SOCK_NONBLOCK, 0 ))) {
        MDMI_LOGE(TAG, "Failed to create domain socket.");
        return false;
    }
    MDMI_LOGD(TAG, "Domain sockect was created. fd = [%d]", fd);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL; /* Socket type is local (Unix Domain). */
    serverNameLen = strlen(szServerName);
    if (serverNameLen > sizeof(servaddr.sun_path) - 1) {
        serverNameLen = sizeof(servaddr.sun_path) - 1;
    }
    memcpy(servaddr.sun_path + 1, szServerName, serverNameLen); /* Define the name of this socket. */
    servaddr.sun_path[0] = 0;

    do {
        size_t addrLen = offsetof(struct sockaddr_un, sun_path) + 1 + serverNameLen;
        if (fd < 0) {
            MDMI_LOGE(TAG, "Failed to check donmain socket");
            break;
        }

        if (0 > bind(fd, (struct sockaddr *)&servaddr, addrLen)) {
            MDMI_LOGE(TAG, "Failed to bind domain socket to [%s]. err = [%s](%d)",
                    szServerName, strerror(errno), errno);
            break;
        }

        if (0 < listen(fd, 4)) {
            MDMI_LOGE(TAG, "Failed to listen domain socket, fd = [%d]. err = [%s](%d)",
                    fd, strerror(errno), errno);
            break;
        }

        AddSocket(SOCKET_TYPE_SERVER, fd, NULL);
        MDMI_LOGD(TAG, "SetupDomainSocket done! Start to listen domain sockect, fd =[%d], file = [%s].",
                fd, servaddr.sun_path + 1);

        return true;
    } while (false);

    if (fd >= 0) {
        close(fd);
    }
    MDMI_LOGE(TAG, "SetupDomainSocket failed.");
    return false;
}

void *SocketListener::ListenThread(void *obj) {
    MDMI_LOGD(TAG, "ListenThread Loop");
    SocketListener *me = reinterpret_cast<SocketListener *> (obj);
    me->Listen();
    pthread_exit(NULL);
    return NULL;
}

#define MAX(a, b) (((a) > (b))? (a): (b))
void SocketListener::Listen() {
    MDMI_LOGD(TAG, "Listen");

    int rc = 0;
    size_t i = 0;
    fd_set read_fds;
    struct timeval timeout;
    std::map<int, SocketConnection *>::iterator socketItr;

    bzero(&timeout, sizeof(timeout));

    MDMI_LOGD(TAG, "Max number of fd = [%d]", m_maxFD);


    if (m_notifyListenThreadStop) {
        return;
    }

    MDMI_LOGD(TAG, "Begins to select.");
    while (!m_notifyListenThreadStop) {
        read_fds = m_masterFDs;

        timeout.tv_sec = 1; // 1 second
        timeout.tv_usec = 0;

        if ((rc = select(m_maxFD + 1, &read_fds, NULL, NULL, &timeout)) < 0) {
            if (errno == EINTR) {
                continue;
            }
            MDMI_LOGE(TAG, "Failed to select, err = [%s](%d). EXIT.", strerror(errno), errno);
            exit(1);
        } else if (0 == rc) {
            // MDMI_LOGD(TAG, "Select timeout. CONTINUE.");
            continue;
        }
        MDMI_LOGD(TAG, "Select done! num of fd = [%d]. max fd = [%d].", rc, m_maxFD);

        std::vector<int> vecInvalidFd;
        /* New connection from server socket*/
        for (socketItr = m_socket[SOCKET_TYPE_SERVER].begin(); socketItr != m_socket[SOCKET_TYPE_SERVER].end(); ++socketItr) {
            int serverFd = socketItr->first;
            SocketConnection &serverConn = *(socketItr->second);

            if (!serverConn.IsValid()) {
                vecInvalidFd.push_back(serverFd);
                continue;
            }
            if (!FD_ISSET(serverFd, &read_fds)) {
                continue;
            }

            struct sockaddr_un addr;
            bzero(&addr, sizeof(addr));
            socklen_t alen = sizeof(addr);
            int connectfd = -1;

            MDMI_LOGD(TAG, "Begins to accept. fd = [%d]", serverFd);

            if (m_notifyListenThreadStop) {
                return;
            }
            if ((connectfd = accept(serverFd, (struct sockaddr *)&addr, &alen)) < 0) {
                MDMI_LOGE(TAG, "Failed to accept from server socket [%d], err = [%s](%d)", serverFd, strerror(errno), errno);
                sleep(1);
                continue;
            }
            int flags = fcntl(connectfd, F_GETFL, 0);
            fcntl(connectfd, F_SETFL, flags | O_NONBLOCK);
            if (m_notifyListenThreadStop) {
                return;
            }
            char szAddr[MAX_SOCK_ADDR_LEN] = {0};
            if (alen - offsetof(struct sockaddr_un, sun_path) > 0) {
                strncpy(szAddr, addr.sun_path, alen - offsetof(struct sockaddr_un, sun_path));
            }
            MDMI_LOGD(TAG, "Accept a client. fd = [%d]", connectfd);
            AddSocket(SOCKET_TYPE_CLIENT, connectfd, szAddr);
            if (m_notifyListenThreadStop) {
                return;
            }
            MDMI_LOGD(TAG, "Accepts one client, fd = [%d] addr = [%s]", connectfd, addr.sun_path+1);
        }
        for (i = 0; i < vecInvalidFd.size(); ++i) {
            RemoveSocket(SOCKET_TYPE_SERVER, vecInvalidFd[i]);
        }
        vecInvalidFd.clear();
        for (socketItr = m_socket[SOCKET_TYPE_CLIENT].begin(); socketItr != m_socket[SOCKET_TYPE_CLIENT].end(); ++socketItr) {
            int clientFd = socketItr->first;
            SocketConnection &clientConn = *(socketItr->second);

            if (!clientConn.IsValid()) {
                vecInvalidFd.push_back(clientFd);
                continue;
            }
            if (!FD_ISSET(clientFd, &read_fds)) {
                continue;
            }
            MDMI_LOGD(TAG, "Start to read command from fd = [%d] addr = [%s]", clientFd, clientConn.GetAddr());
            if (m_notifyListenThreadStop) {
                return;
            }
            /**
             * @todo Add lock to protect m_connHandler & m_handlerPara
             */
            if (m_connHandlerInstance) {
                m_connHandlerInstance->DataIn(clientConn);
                if (m_notifyListenThreadStop) {
                    return;
                }
            }
            if (m_notifyListenThreadStop) {
                return;
            }
        }
        for (i = 0; i < vecInvalidFd.size(); ++i) {
            RemoveSocket(SOCKET_TYPE_CLIENT, vecInvalidFd[i]);
        }
        if (m_notifyListenThreadStop) {
            return;
        }
    }
}
