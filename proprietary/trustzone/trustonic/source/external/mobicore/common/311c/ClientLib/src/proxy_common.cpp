/*
 * Copyright (c) 2013-2016 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#undef LOG_TAG
#define LOG_TAG "TeeSyProxyCommon"
#include "log.h"
#include "proxy_common.h"

using namespace com::trustonic::tee_proxy;

CommonConnection::~CommonConnection() {
    LOG_D("socket %d disconnected", sock_);
    if (sock_ >= 0) {
        ::close(sock_);
    }
}

int CommonConnection::connect() {
    sock_ = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_ < 0) {
        LOG_ERRNO("socket");
        return -1;
    }
    struct sockaddr_un sock_un;
    sock_un.sun_family = AF_UNIX;
    ::strncpy(sock_un.sun_path, SOCKET_PATH, sizeof(sock_un.sun_path) - 1);
    socklen_t len = static_cast<socklen_t>(strlen(sock_un.sun_path) +
                                           sizeof(sock_un.sun_family));
    sock_un.sun_path[0] = '\0';         // Abstract
    if (::connect(sock_, reinterpret_cast<struct sockaddr*>(&sock_un),
                  len) < 0) {
        LOG_ERRNO("connect");
        return -1;
    }
    LOG_D("socket %d connected", sock_);
    return 0;
}

int CommonConnection::getClientCredentials() {
    struct ucred credentials;
    socklen_t len = sizeof(credentials);

    if (::getsockopt(sock_, SOL_SOCKET, SO_PEERCRED, &credentials, &len)) {
        LOG_ERRNO("getsockopt");
    }

    if (credentials.pid == 0) {
        errno = EAGAIN;
        LOG_ERRNO("pid");
        return -1;
    }

    pid_ = credentials.pid;
    uid_ = credentials.uid;
    LOG_I("PID: %u UID: %d", pid_, uid_);
    return 0;
}

int CommonConnection::send(
        const char* what,
        const void* buffer,
        size_t length,
        bool* signal_received) {
    auto cbuffer = static_cast<const char*>(buffer);
    size_t count = 0;

    while (count < length) {
        ssize_t rc = ::send(sock_, &cbuffer[count], length - count,
                            MSG_NOSIGNAL);
        switch (rc) {
            case -1:
                if (errno != EINTR) {
                    LOG_ERRNO(what);
                    return -1;
                }
                if (signal_received) {
                    LOG_W("signal received, will be reported, keep going");
                    *signal_received = true;
                } else {
                    LOG_W("signal received, ignored, keep going");
                }
                break;
            case 0:
                LOG_E("socket closed while sending %s", what);
                return -1;
            default:
                count += rc;
        }
    }
    return 0;
}

int CommonConnection::receive(
        const char* what,
        void* buffer,
        size_t length,
        bool* signal_received,
        bool may_close) {
    auto cbuffer = static_cast<char*>(buffer);
    size_t count = 0;

    while (count < length) {
        ssize_t rc = ::recv(sock_, &cbuffer[count], length - count, 0);
        switch (rc) {
            case -1:
                if (errno != EINTR) {
                    LOG_ERRNO(what);
                    return -1;
                }
                if (signal_received) {
                    LOG_W("signal received, will be reported, keep going");
                    *signal_received = true;
                } else {
                    LOG_W("signal received, ignored, keep going");
                }
                break;
            case 0:
                if (may_close) {
                    LOG_D("socket closed");
                } else {
                    LOG_E("socket closed while receiving %s", what);
                }
                return -1;
            default:
                count += rc;
        }
    }
    return 0;
}

int CommonConnection::sendRequest(
        enum RpcMethod method,
        const std::string& buffer,
        bool* signal_received) {
    // Send request header
    RequestHeader header;
    ::memcpy(header.magic, PROTOCOL_MAGIC, sizeof(header.magic));
    header.version = static_cast<uint16_t>(PROTOCOL_VERSION);
    header.method = static_cast<uint16_t>(method);
    header.length = static_cast<uint32_t>(buffer.length());
    if (send("header", &header, sizeof(header), signal_received)) {
        return -1;
    }

    // Send request data
    LOG_D("send %u bytes of data", header.length);
    if (send("data", &buffer[0], header.length, signal_received)) {
        return -1;
    }

    return 0;
}

int CommonConnection::receiveRequest(
        enum RpcMethod* method,
        std::string* buffer,
        bool* signal_received) {
    // Receive request header
    RequestHeader header;
    if (receive("header", &header, sizeof(header), signal_received, true)) {
        return -1;
    }
    if (::memcmp(header.magic, PROTOCOL_MAGIC, sizeof(header.magic))) {
        LOG_E("Wrong magic");
        return -1;
    }
    if (header.version != PROTOCOL_VERSION) {
        LOG_E("Wrong version");
        return -1;
    }
    *method = static_cast<RpcMethod>(header.method);
    buffer->resize(header.length);
    LOG_D("receive %u bytes of data for method %s", header.length,
          methodToString(*method));

    // Receive request data
    if (receive("data", &(*buffer)[0], header.length, signal_received)) {
        return -1;
    }

    return header.length;
}

int CommonConnection::sendResponse(
        const std::string& buffer,
        int proto_rc,
        int method_rc,
        bool* signal_received) {
    // Send response header
    ResponseHeader header;
    ::memcpy(header.magic, PROTOCOL_MAGIC, sizeof(header.magic));
    header.version = static_cast<uint16_t>(PROTOCOL_VERSION);
    header.reserved = 0;
    header.proto_rc = proto_rc;
    header.method_rc = method_rc;
    if (send("header", &header, sizeof(header), signal_received)) {
        return -1;
    }

    // Send response data
    if (header.proto_rc > 0) {
        LOG_D("send %d bytes of data", header.proto_rc);
        if (send("data", &buffer[0], header.proto_rc, signal_received)) {
            return -1;
        }
    }

    return 0;
}

int CommonConnection::receiveResponse(
        std::string* buffer,
        int* method_rc,
        bool* signal_received) {
    // Receive response header
    ResponseHeader header;
    if (receive("header", &header, sizeof(header), signal_received)) {
        return -1;
    }

    // Check header
    if (::memcmp(header.magic, PROTOCOL_MAGIC, sizeof(header.magic))) {
        LOG_E("Wrong magic");
        return -1;
    }
    if (header.version != PROTOCOL_VERSION) {
        LOG_E("Wrong version");
        return -1;
    }
    if (header.proto_rc < 0) {
        errno = -header.proto_rc;
        if (errno == EAGAIN) {
            LOG_W("Protocol error reported: %s", strerror(errno));
        } else {
            LOG_E("Protocol error reported: %s", strerror(errno));
        }
        return -1;
    }

    // Receive response data
    LOG_D("receive %d bytes of data", header.proto_rc);
    if (header.proto_rc > 0) {
        // Receive response data
        int length = header.proto_rc;
        buffer->resize(length);
        if (receive("data", &(*buffer)[0], length, signal_received)) {
            return -1;
        }
    }

    *method_rc = header.method_rc;
    return header.proto_rc;
}
