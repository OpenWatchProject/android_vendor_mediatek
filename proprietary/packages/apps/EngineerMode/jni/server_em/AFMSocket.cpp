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

// AFMSocket.cpp: implementation of the AFMSocket class.
#define LOG_TAG "AFMSocket"

#define MTK_LOG_ENABLE 1
#include <cutils/log.h>
#include "AFMSocket.h"

static int s_read_thread_result = 0;

void* do_SocketReadRun(AFMSocket* socket) {
    pthread_detach(pthread_self());
    if (socket) {
        s_read_thread_result = socket->DoReadRun();
    } else {
        s_read_thread_result = -1;
    }
    return &s_read_thread_result;
}

AFMSocket::AFMSocket() {
    m_thr = NULL;
    m_hSocket = INVALID_SOCKET;
    m_SocketCanSend = true;

}

bool AFMSocket::Create(unsigned int nSocketPort, int nSocketType, LPCTSTR lpszSocketAddress) {
#if defined(USE_LOCAL_SOCKET)
    ALOGD("nSocketPort = %d\n", nSocketPort);
    m_hSocket = socket(AF_LOCAL, nSocketType, 0);
    if (m_hSocket == INVALID_SOCKET) {
        ALOGD("AFMSocket::socket AF_LOCAL failure\n");
        return false;
    }

    if (lpszSocketAddress != NULL) {  // server socket.
        SOCKADDR_UN sockAddr;
        memset(&sockAddr, 0, sizeof(sockAddr));
        sockAddr.sun_family = AF_LOCAL;

        sockAddr.sun_path[0] = 0;
        memcpy(sockAddr.sun_path + 1, lpszSocketAddress, strlen(lpszSocketAddress));
        int n = 1;
        if (setsockopt(m_hSocket, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n))
                != 0) {
            ALOGD("setsockopt failed with error %s\n", strerror(errno));
            Close();
            return false;
        }

        int offset = (char *)&(sockAddr.sun_path) - (char *)&sockAddr;

        int len = strlen(lpszSocketAddress) + offset + 1;

        if (bind(m_hSocket, (SOCKADDR*)&sockAddr, len)) {
            ALOGD("AFMSocket::bind failure\n addr = %s, family = %d, AF_LOCAL=%d sun_path size %d\n",
                    sockAddr.sun_path, sockAddr.sun_family, AF_LOCAL, (int)sizeof(sockAddr.sun_path));
            Close();
            return false;
        }
    }
    return true;
#else
    m_hSocket = socket(AF_INET, nSocketType, 0);
    if (m_hSocket == INVALID_SOCKET)
        return false;

    SOCKADDR_IN sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));

    sockAddr.sin_family = AF_INET;

    if (lpszSocketAddress == NULL)
        sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    else {
        int lResult = inet_addr(lpszSocketAddress);
        if (lResult == (int) INADDR_NONE) {
            return false;
        }
        sockAddr.sin_addr.s_addr = lResult;
    }

    sockAddr.sin_port = htons((u_short) nSocketPort);

    if (bind(m_hSocket, (SOCKADDR*) &sockAddr, sizeof(sockAddr))) {
        Close();
        return false;
    }

    return true;
#endif
}

bool AFMSocket::Accept(AFMSocket& rConnectedSocket, SOCKADDR* lpSockAddr, int* lpSockAddrLen) {
    SOCKET hTemp = accept(m_hSocket, lpSockAddr, (socklen_t*) lpSockAddrLen);

    if (hTemp == INVALID_SOCKET) {
        rConnectedSocket.m_hSocket = INVALID_SOCKET;
        return false;
    } else {
        m_SocketCanSend = true;
        rConnectedSocket.m_hSocket = hTemp;

        int cmd = 0;
        rConnectedSocket.IOCtl(FIONBIO, &cmd);

        rConnectedSocket.m_thr = new AFMThread();
        if (rConnectedSocket.m_thr == 0) {
            return false;
        }
        rConnectedSocket.m_thr->Start((AFM_THREADFUNC) do_SocketReadRun, &rConnectedSocket);
        return true;
    }
}

void AFMSocket::Close() {
    m_SocketCanSend = false;
    if (m_hSocket != INVALID_SOCKET) {
        ShutDown(SHUT_RDWR);
        m_Lock.Lock();
        close(m_hSocket);

        m_hSocket = INVALID_SOCKET;
        m_Lock.Unlock();
    }
}

bool AFMSocket::IOCtl(long lCommand, int* lpArgument) {

    if (ioctl(m_hSocket, lCommand, lpArgument) == -1)
        return false;
    else
        return true;

}

int AFMSocket::Receive(void* lpBuf, int nBufLen, int nFlags) {
    if (m_hSocket == INVALID_SOCKET)
        return -1;
    return recv(m_hSocket, (char*) lpBuf, nBufLen, nFlags);
}

bool AFMSocket::ShutDown(int nHow) {
    return !shutdown(m_hSocket, nHow);
}

int AFMSocket::Send(const void* lpBuf, int nBufLen, int nFlags) {

    nFlags |= MSG_NOSIGNAL;   // prevent process dead.

    AFMSingleLock lock(&m_WriteLock);
    lock.Lock();

    if (m_hSocket == INVALID_SOCKET || m_SocketCanSend != true) {
        return -1;
    }
    int ret = 0;
    int offset = 0;
    fd_set rset;
    do {
        FD_ZERO(&rset);
        FD_SET(m_hSocket, &rset);
        ret = send(m_hSocket, (char*) lpBuf + offset, nBufLen - offset, nFlags);
        ALOGD("raw send ret %d\n", ret);
        if (ret < 0) {
            return ret;
        }
        offset += ret;
        if (offset == nBufLen) {
            return offset;
        }
    } while (1);
    return ret;
}

void AFMSocket::OnReceive(int /*nErrorCode*/) {
}

void AFMSocket::OnSend(int /*nErrorCode*/) {
}

void AFMSocket::OnAccept(int /*nErrorCode*/) {
}

void AFMSocket::OnConnect(int /*nErrorCode*/) {
}

void AFMSocket::OnClose(int /*nErrorCode*/) {
}

int AFMSocket::DoReadRun() {
    char buf[1];
    int nErrorCode;
    fd_set rset;
    struct timeval wtime;
    int rc;

    while (m_thr->Running()) {
        m_Lock.Lock();
        if (m_hSocket == INVALID_SOCKET) {
            m_Lock.Unlock();
            break;
        }

        wtime.tv_sec = 0;
        wtime.tv_usec = 200 * 1000;

        FD_ZERO(&rset);
        FD_SET(m_hSocket, &rset);

        rc = select(m_hSocket + 1, &rset, NULL, NULL, &wtime);

        if (rc == 0) {
            m_Lock.Unlock();
            continue;
        }

        nErrorCode = 0;
        rc = recv(m_hSocket, buf, 1, MSG_PEEK);

        if (rc < 0) {
            m_SocketCanSend = false;
            nErrorCode = errno;

            m_Lock.Unlock();
            OnClose(nErrorCode);
            break;
        } else if (rc == 0) {
            m_Lock.Unlock();
            OnClose(0);
            break;
        } else {
            m_Lock.Unlock();
            OnReceive(0);
        }
    }

    delete this;

    return 0;
}

void AFMSocket::StopAndDelete() {
    m_thr->NotifyStop();
}

AFMSocket::~AFMSocket() {
    m_thr->NotifyStop();

    if (m_hSocket != INVALID_SOCKET)
        Close();

    if (m_thr) {
        delete m_thr;
        m_thr = NULL;
    }
}

