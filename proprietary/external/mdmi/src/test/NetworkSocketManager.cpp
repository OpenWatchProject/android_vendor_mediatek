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

#include "NetworkSocketManager.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include "Mdmi_utils.h"

#undef TAG
#define TAG "MDMI-NetworkSocketManager"

NetworkSocketManager::NetworkSocketManager(size_t port):
        m_maxFD(-1), m_threadId(pthread_self()), m_notifyListenThreadStop(false) {
    FD_ZERO(&m_masterFDs);
    if (!SetupNetworkSocket(port)) {
        MDMI_LOGE(TAG, "Fail to setup network socket.");
        Cleanup();
        return;
    }
    pthread_mutex_init(&m_socketLock, NULL);
    if (pthread_create(&m_threadId, NULL, NetworkSocketManager::ListenThread, this)) {
        MDMI_LOGD(TAG, "pthread_create (%s)", strerror(errno));
        Cleanup();
    }
}

NetworkSocketManager::~NetworkSocketManager()
{
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
    pthread_mutex_destroy(&m_socketLock);
}

bool NetworkSocketManager::AddSocket(SOCKET_TYPE type, int fd)
{
    MDMI_LOGD(TAG, "AddSocket fd = [%d]", fd);
    std::map<int, NetworkSocketConnection *>::iterator itr;
    if ((itr = m_socket[type].find(fd)) == m_socket[type].end()) {
        NetworkSocketConnection *pConn = NULL;
        if (NULL == (pConn = (NetworkSocketConnection *)new NetworkSocketConnection(fd))) {
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

void NetworkSocketManager::RemoveSocket(SOCKET_TYPE type, int fd)
{
    MDMI_LOGD(TAG, "RemoveSocket fd = [%d]", fd);
    std::map<int, NetworkSocketConnection *>::iterator itr;
    pthread_mutex_lock(&m_socketLock);
    if ((itr = m_socket[type].find(fd)) != m_socket[type].end()) {
        delete itr->second;
        m_socket[type].erase(itr);
    }
    pthread_mutex_unlock(&m_socketLock);
    FD_CLR(fd, &m_masterFDs);
}

void NetworkSocketManager::Cleanup()
{
    pthread_mutex_lock(&m_socketLock);
    for (int i = 0; i < SOCKET_TYPE_SIZE; ++i) {
        std::map<int, NetworkSocketConnection *>::iterator socketItr;
        for (socketItr = m_socket[i].begin(); socketItr != m_socket[i].end(); ++socketItr) {
            delete socketItr->second;
        }
        m_socket[i].clear();
    }
    pthread_mutex_unlock(&m_socketLock);
}

bool NetworkSocketManager::SetupNetworkSocket(size_t port)
{
    int fd = -1;
    struct sockaddr_in servaddr;

    MDMI_LOGD(TAG, "NetworkSocketManager::SetupNetworkSocket [%zu]", port);

    if (0 > (fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0 ))) {
        MDMI_LOGE(TAG, "Failed to create network socket.");
        return false;
    }
    MDMI_LOGD(TAG, "Network sockect was created. fd = [%d]", fd);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = 0;
    servaddr.sin_port = htons(port);

    do {
        int yes = 1;

        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) {
            MDMI_LOGE(TAG, "NetworkSocketManager::SetupNetworkSocket setsockopt error");
            break;
        }

        if (0 > bind(fd, (struct sockaddr *)&servaddr, sizeof(servaddr))) {
            MDMI_LOGE(TAG, "NetworkSocketManager::SetupNetworkSocket Failed to bind network socket on port [%zu]. err = [%s](%d)", port, strerror(errno), errno);
            break;
        }

        if (0 < listen(fd, 4)) {
            MDMI_LOGE(TAG, "NetworkSocketManager::SetupNetworkSocket Failed to listen network socket, fd = [%d]. err = [%s](%d)", fd, strerror(errno), errno);
            break;
        }

        AddSocket(SOCKET_TYPE_SERVER, fd);
        MDMI_LOGI(TAG, "NetworkSocketManager::SetupNetworkSocket done! Start to listen network sockect, fd =[%d], port = [%zu].", fd, port);

        return true;
    } while (false);

    if (fd >= 0) {
        close(fd);
    }
    MDMI_LOGE(TAG, "NetworkSocketManager::SetupNetworkSocket failed.");
    return false;
}

void *NetworkSocketManager::ListenThread(void *obj)
{
    MDMI_LOGI(TAG, "NetworkSocketManager::ListenThread Loop");
    NetworkSocketManager *me = reinterpret_cast<NetworkSocketManager *> (obj);
    me->Listen();
    pthread_exit(NULL);
    return NULL;
}

#define MAX(a, b) (((a) > (b))? (a): (b))
void NetworkSocketManager::Listen()
{
    MDMI_LOGD(TAG, "NetworkSocketManager::Listen");

    int rc = 0;
    size_t i = 0;
    fd_set read_fds;
    struct timeval timeout;
    std::map<int, NetworkSocketConnection *>::iterator socketItr;

    bzero(&timeout, sizeof(timeout));

    MDMI_LOGD(TAG, "Max number of fd = [%d]", m_maxFD);


    if (m_notifyListenThreadStop) {
        return;
    }

    MDMI_LOGI(TAG, "NetworkSocketManager begins to select.");
    while (!m_notifyListenThreadStop)
    {
        read_fds = m_masterFDs;

        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; // 0.1 second

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
        MDMI_LOGI(TAG, "NetworkSocketManager select done! num of fd = [%d]. max fd = [%d].", rc, m_maxFD);

        std::vector<int> vecInvalidFd;
        /* New connection from server socket*/
        pthread_mutex_lock(&m_socketLock);
        for (socketItr = m_socket[SOCKET_TYPE_SERVER].begin(); socketItr != m_socket[SOCKET_TYPE_SERVER].end(); ++socketItr) {
            int serverFd = socketItr->first;
            NetworkSocketConnection &serverConn = *(socketItr->second);

            if (!serverConn.IsValid()) {
                vecInvalidFd.push_back(serverFd);
                continue;
            }
            if (!FD_ISSET(serverFd, &read_fds)) {
                continue;
            }

            struct sockaddr_in addr;
            bzero(&addr, sizeof(addr));
            socklen_t alen = sizeof(addr);
            int connectfd = -1;

            MDMI_LOGI(TAG, "NetworkSocketManager begins to accept. fd = [%d]", serverFd);

            if (m_notifyListenThreadStop) {
                pthread_mutex_unlock(&m_socketLock);
                return;
            }
            if ((connectfd = accept(serverFd, (struct sockaddr *)&addr, &alen)) < 0) {
                MDMI_LOGE(TAG, "Failed to accept from server socket [%d], err = [%s](%d)", serverFd, strerror(errno), errno);
                sleep(1);
                continue;
            }
            if (m_notifyListenThreadStop) {
                pthread_mutex_unlock(&m_socketLock);
                return;
            }
            MDMI_LOGI(TAG, "Accept a client. fd = [%d]", connectfd);
            AddSocket(SOCKET_TYPE_CLIENT, connectfd);
            if (m_notifyListenThreadStop) {
                pthread_mutex_unlock(&m_socketLock);
                return;
            }
            MDMI_LOGI(TAG, "NetworkSocketManager accepts one client, fd = [%d]", connectfd);
        }
        pthread_mutex_unlock(&m_socketLock);
        for (i = 0; i < vecInvalidFd.size(); ++i) {
            RemoveSocket(SOCKET_TYPE_SERVER, vecInvalidFd[i]);
        }
        vecInvalidFd.clear();
        pthread_mutex_lock(&m_socketLock);
        for (socketItr = m_socket[SOCKET_TYPE_CLIENT].begin(); socketItr != m_socket[SOCKET_TYPE_CLIENT].end(); ++socketItr) {
            int clientFd = socketItr->first;
            NetworkSocketConnection &clientConn = *(socketItr->second);

            if (!clientConn.IsValid()) {
                vecInvalidFd.push_back(clientFd);
                continue;
            }
            if (!FD_ISSET(clientFd, &read_fds)) {
                continue;
            }
            MDMI_LOGD(TAG, "Start to read command from fd = [%d]", clientFd);
            if (m_notifyListenThreadStop) {
                pthread_mutex_unlock(&m_socketLock);
                return;
            }
            /**
             * @todo Add lock to protect m_connHandler & m_handlerPara
             */
            clientConn.Drain();
            if (!clientConn.IsValid()) {
                m_notifyListenThreadStop = true;
                pthread_mutex_unlock(&m_socketLock);
                MDMI_LOGD(TAG, "NetworkSocketManager::Listen exiting redirector");
                return;
            }
#if 0
            if (m_connHandlerInstance) {
                m_connHandlerInstance->DataIn(clientConn);
                if (m_notifyListenThreadStop) {
                    return;
                }
            }
#endif
            if (m_notifyListenThreadStop) {
                pthread_mutex_unlock(&m_socketLock);
                return;
            }
        }
        pthread_mutex_unlock(&m_socketLock);
        for (i = 0; i < vecInvalidFd.size(); ++i) {
            RemoveSocket(SOCKET_TYPE_CLIENT, vecInvalidFd[i]);
        }
        if (m_notifyListenThreadStop) {
            return;
        }
    }
}

bool NetworkSocketManager::Write(const uint8_t *data, size_t len) {
    std::map<int, NetworkSocketConnection *>::iterator socketItr;
    pthread_mutex_lock(&m_socketLock);
    for (socketItr = m_socket[SOCKET_TYPE_CLIENT].begin(); socketItr != m_socket[SOCKET_TYPE_CLIENT].end(); ++socketItr) {
        if (NULL == socketItr->second) {
            continue;
        }
        if (socketItr->second->IsValid()) {
            bool ret = socketItr->second->Write(data, len, NULL);
            if (!ret && !socketItr->second->IsValid()) {
                //adb disconnected
                if (!m_notifyListenThreadStop) {
                    pthread_mutex_unlock(&m_socketLock);
                    MDMI_LOGD(TAG, "NetworkSocketManager::Write exiting redirector");
                }
                return false;
            }
        }
    }
    pthread_mutex_unlock(&m_socketLock);
    return true;
}
