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

#include "SocketConnection.h"
#include "Mdmi_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stddef.h>     /* offsetof */
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>/* for Unix domain sockets */
#include <sys/poll.h>

static const char *TAG = "MDMI-SocketConnection";

/************* SocketConnection *************/
SocketConnection::SocketConnection():m_isValid(false), m_fd(-1) {
    bzero(&m_addr, sizeof(m_addr));
}

SocketConnection::SocketConnection(int fd, const char *szAddr):m_isValid(true), m_fd(fd) {
    if (NULL != szAddr) {
        snprintf(m_addr, sizeof(m_addr), "%s", szAddr);
    }
}

SocketConnection::~SocketConnection() {
    MDMI_LOGD(TAG, "~SocketConnection fd = [%d]", m_fd);
    Reset();
}

void SocketConnection::SetAddr(const char *szAddr) {
    if (NULL != szAddr) {
        snprintf(m_addr, sizeof(m_addr), "%s", szAddr);
    }
}

void SocketConnection::Reset() {
    if (m_fd >= 0) {
        close(m_fd);
        m_fd = -1;
    }
	m_isValid = false;
    bzero(&m_addr, sizeof(m_addr));
}

bool SocketConnection::Connect(void *host) {
    struct sockaddr_un servaddr;
    int fd = -1;
    char szServerName[MAX_SOCK_ADDR_LEN] = {0};
    size_t serverNameLen = 0;

    MDMI_LOGD(TAG, "Connect");

    if (NULL == host) {
        MDMI_LOGE(TAG, "Input arg is NULL");
        return false;
    }
    Reset();
    snprintf(szServerName, sizeof(szServerName), "%s", (const char *)host);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL; /* Socket type is local (Unix Domain). */
    serverNameLen = strlen(szServerName);
    if (serverNameLen > sizeof(servaddr.sun_path) - 1) {
        serverNameLen = sizeof(servaddr.sun_path) - 1;
    }
    memcpy(servaddr.sun_path + 1, szServerName, serverNameLen); /* Define the name of this socket. */
    servaddr.sun_path[0] = 0;

    do {
        if (0 > (fd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0))) {
            MDMI_LOGE(TAG, "Failed to establish domain socket.");
            break;
        }

        size_t addrLen = offsetof(struct sockaddr_un, sun_path) + 1 + serverNameLen;
        if (0 > connect(fd, (struct sockaddr *)(&servaddr), addrLen)) {
            MDMI_LOGE(TAG, "Failed to connect to [%s], err = [%s]", szServerName, strerror(errno));
            break;
        }
        m_fd = fd;
        snprintf(m_addr, sizeof(m_addr), "%s", szServerName);
        m_isValid = true;
        MDMI_LOGD(TAG, "Init done!");
        return true;
    } while (false);

    if (fd >= 0) {
        close(fd);
    }
    MDMI_LOGE(TAG, "Init failed.");
    return false;
}

size_t SocketConnection::Read(uint8_t *buffer, size_t bufferSize) {
    ssize_t readBytes = -1;
    int retry = 0;
#define READ_RETRY_TIMES 5
    MDMI_LOGD(TAG, "SocketConnection::Read");

    if (NULL == buffer || 0 == bufferSize) {
        MDMI_LOGE(TAG, "Bad parameters. buffer = [%p], bufferSize = [%zu]", buffer, bufferSize);
        return -2;
    }

    while (true) {
        if (0 > (readBytes = read(m_fd, buffer, bufferSize))) {
            if (errno == EAGAIN || errno == EINTR) {
                if (retry == READ_RETRY_TIMES) {
                    MDMI_LOGD(TAG, "No data to read from fd: [%d], retry [%d] times when err = [%s](%d)",
                            m_fd, retry, strerror(errno), errno);
                    return 0;
                }
                ++retry;
                continue;
            }
            MDMI_LOGE(TAG, "Failed to read from fd: [%d], err = [%s](%d)", m_fd, strerror(errno), errno);
            return -1;
        } else if (0 == readBytes) {
            MDMI_LOGD(TAG, "Socket is closed. fd: [%d]", m_fd);
            m_isValid = false;
            return -1;
        }
        MDMI_LOGD(TAG, "Read done! read [%zd] bytes.", readBytes);
        return readBytes;
    }
}

bool SocketConnection::Write(const uint8_t *data, size_t len, volatile const bool *bStopFlag) {
    ssize_t writtenBytes = -1;
    size_t totalBytes = 0;

    MDMI_LOGD(TAG, "Write to fd: [%d] size=[%zu]", m_fd, len);

    if (data == NULL) {
        return false;
    }

    if (len == 0) {
        return true;
    }

    if (!IsValid()) {
        return false;
    }

    MDMI_LOGD(TAG, "write to fd: [%d]", m_fd);

    while (totalBytes < len) {
        if (NULL != bStopFlag && *bStopFlag) {
            return true;
        }
        if (0 > (writtenBytes = write(m_fd, data + totalBytes, len - totalBytes))) {
            if (errno == EAGAIN || errno == EINTR) {
                if (NULL != bStopFlag && *bStopFlag) {
                    return true;
                }
                continue;
            }
            MDMI_LOGE(TAG, "Failed to write to fd: [%d], err = [%s](%d)", m_fd, strerror(errno), errno);
            m_isValid = false;
            return false;
        }
        totalBytes += writtenBytes;
    }

    MDMI_LOGD(TAG, "Send to fd: [%d] size=[%zu] successfully.", m_fd, len);
    return true;
}

bool SocketConnection::CheckValid() {
    if (!IsValid()) {
        MDMI_LOGE(TAG, "socket invalid. fd = %d, err = %s", m_fd, strerror(errno));
        return false;
    }
    struct pollfd event;
    memset(&event, 0, sizeof(event));
    event.fd = m_fd;
    event.events = POLLIN | POLLOUT;
    int ret_poll = poll((struct pollfd *)&event, 1, 0);
    if (ret_poll == 0) {
        MDMI_LOGE(TAG, "no event happen");
        return true;
    }
    if (ret_poll < 0) {
        if (errno == EAGAIN || errno == EINTR) {
            MDMI_LOGD(TAG, "socket valid, just no data to read from fd = %d, err = %s",
                    m_fd, strerror(errno));
            return true;
        }
        MDMI_LOGE(TAG, "poll error, fd = %d, err = %s", m_fd, strerror(errno));
        m_isValid = false;
        return false;
    }
    if (event.revents & POLLHUP || event.revents & POLLNVAL || event.revents & POLLERR) {
        MDMI_LOGE(TAG, "socket invalid! revents = 0x%x", event.revents);
        m_isValid = false;
        return false;
    }
    MDMI_LOGE(TAG, "socket valid! revents = 0x%x", event.revents);
    return true;
}

bool SocketConnection::ReadToBuffer() {
    ssize_t readBytes = -1;

    MDMI_LOGD(TAG, "ReadToBuffer. There're [%zu] bytes of data in dataQ.", m_dataQ.Size());

    if (0 > (readBytes = Read(m_dataQ.SpaceStart(), m_dataQ.SpaceSize()))) {
        MDMI_LOGD(TAG, "Failed to read from connection");
        return false;
    }
    m_dataQ.IncSize(readBytes);

    MDMI_LOGD(TAG, "ReadToBuffer done! read [%zd] bytes.", readBytes);
    return true;
}
