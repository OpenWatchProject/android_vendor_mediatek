/*
 * Copyright (c) 2013-2018 TRUSTONIC LIMITED
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

#include "dynamic_log.h"
#include "proxy_common.h"

#define LOG_MESSAGE(type, method, what, text) \
    do { \
        std::string message(methodToString(method)); \
        message += " ("; \
        message += what; \
        message += "): "; \
        message += text; \
        switch (type) { \
            case 'E': \
                LOG_E("%s", message.c_str()); \
                break; \
            case 'W': \
                LOG_W("%s", message.c_str()); \
                break; \
            case 'I': \
                LOG_I("%s", message.c_str()); \
                break; \
            case 'D': \
                LOG_D("%s", message.c_str()); \
                break; \
            default: \
                LOG_ERRNO(message.c_str()); \
                break; \
        } \
    } while (0)

using namespace trustonic;

CommonConnection::~CommonConnection() {
    if (sock_ >= 0) {
        close();
    }
}

int CommonConnection::listen() {
    int sock = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        LOG_ERRNO("socket");
        return -1;
    }
    struct sockaddr_un sock_un;
    sock_un.sun_family = AF_UNIX;
    ::strncpy(sock_un.sun_path, path_.c_str(), sizeof(sock_un.sun_path) - 1);
    socklen_t len = static_cast<socklen_t>(strlen(sock_un.sun_path) +
                                           sizeof(sock_un.sun_family));
    sock_un.sun_path[0] = '\0';         // Abstract
    if (::bind(sock, reinterpret_cast<struct sockaddr*>(&sock_un), len) < 0) {
        LOG_ERRNO("bind");
    } else if (::listen(sock, 5) < 0) {
        LOG_ERRNO("listen");
    } else {
        sock_ = sock;
        is_server_ = true;
        LOG_I("server open");
        return 0;
    }
    ::close(sock);
    return -1;
}

std::unique_ptr<CommonConnection> CommonConnection::accept() {
    int sock = ::accept(sock_, nullptr, nullptr);
    if (sock < 0) {
        LOG_ERRNO("accept");
        return nullptr;
    }
    LOG_D("server socket %d connected", sock);
    std::unique_ptr<CommonConnection> conn(new CommonConnection(sock, version_));
    if (conn && ((conn->receiveCredentials() < 0) ||
                 (conn->getClientCredentials() < 0))) {
        conn.reset();
    }
    return conn;
}

int CommonConnection::connect() {
    sock_ = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_ < 0) {
        return -1;
    }
    struct sockaddr_un sock_un;
    sock_un.sun_family = AF_UNIX;
    ::strncpy(sock_un.sun_path, path_.c_str(), sizeof(sock_un.sun_path) - 1);
    socklen_t len = static_cast<socklen_t>(strlen(sock_un.sun_path) +
                                           sizeof(sock_un.sun_family));
    sock_un.sun_path[0] = '\0';         // Abstract
    if (::connect(sock_, reinterpret_cast<struct sockaddr*>(&sock_un),
                  len) < 0) {
        LOG_D("client socket connect failed with %s", strerror(errno));
        return -1;
    }
    if (sendCredentials() < 0) {
        return -1;
    }
    LOG_D("client socket %d connected", sock_);
    return 0;
}

void CommonConnection::close() {
    ::close(sock_);
    LOG_D("%s socket %d closed", is_server_ ? "server" : "client", sock_);
    sock_ = -1;
    is_server_ = false;
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

int CommonConnection::sendCredentials() {
    struct iovec iov;
    uint8_t data = 0;
    iov.iov_base = &data;
    iov.iov_len  = sizeof(data);

    struct msghdr msg = { 0, 0, &iov, 1, nullptr, 0, 0 };
    do {
        if (::sendmsg(sock_, &msg, 0) == sizeof(data)) {
            break;
        }
        if ((errno != EINTR) && (errno != EAGAIN)) {
            LOG_D("%s failed with %s", __func__, strerror(errno));
            return -1;
        }
    } while (true);
    return 0;
}

int CommonConnection::receiveCredentials() {
    // This method is now just a fake as getClientCredentials supersedes it
    struct iovec iov;
    uint8_t data;
    iov.iov_base = &data;
    iov.iov_len  = sizeof(data);

    struct msghdr msg = { 0, 0, &iov, 1, nullptr, 0, 0 };
    do {
        if (::recvmsg(sock_, &msg, 0) == sizeof(data)) {
            break;
        }
        if ((errno != EINTR) && (errno != EAGAIN)) {
            LOG_ERRNO("recvmsg");
            return -1;
        }
    } while (true);
    return 0;
}

int CommonConnection::send(
    RpcMethod method,
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
                    LOG_MESSAGE(' ', method, what, "send");
                    return -1;
                }
                if (signal_received) {
                    LOG_MESSAGE('W', method, what, "signal received, will be reported, keep going");
                    *signal_received = true;
                } else {
                    LOG_MESSAGE('W', method, what, "signal received, ignored, keep going");
                }
                break;
            case 0:
                LOG_MESSAGE('E', method, what, "socket closed while sending");
                return -1;
            default:
                count += rc;
        }
    }
    return 0;
}

int CommonConnection::receive(
    RpcMethod method,
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
                    LOG_MESSAGE('W', method, what, "signal received, will be reported, keep going");
                    *signal_received = true;
                } else {
                    LOG_MESSAGE('W', method, what, "signal received, ignored, keep going");
                }
                break;
            case 0:
                if (may_close) {
                    LOG_MESSAGE('D', method, what, "socket closed");
                } else {
                    LOG_MESSAGE('E', method, what, "socket closed while receiving");
                }
                return -1;
            default:
                count += rc;
        }
    }
    return 0;
}

int CommonConnection::sendRequest(
    RpcMethod method,
    const std::string& buffer,
    bool* signal_received) {
    // Send request header
    RequestHeader header;
    ::memcpy(header.magic, PROTOCOL_MAGIC, sizeof(header.magic));
    header.version = static_cast<uint16_t>(version_);
    header.method = static_cast<uint16_t>(method);
    header.length = static_cast<uint32_t>(buffer.length());
    if (send(method, "header", &header, sizeof(header), signal_received)) {
        return -1;
    }

    // Send request data
    LOG_D("send %u bytes of data", header.length);
    if (send(method, "data", &buffer[0], header.length, signal_received)) {
        return -1;
    }

    return 0;
}

int CommonConnection::receiveRequest(
    RpcMethod* method,
    std::string* buffer,
    bool* signal_received) {
    // Receive request header
    RequestHeader header;
    if (receive(UNKNOWN, "header", &header, sizeof(header), signal_received, true)) {
        return -1;
    }

    // Check magic
    if (::memcmp(header.magic, PROTOCOL_MAGIC, sizeof(header.magic))) {
        LOG_E("%s: wrong magic", methodToString(UNKNOWN));
        LOG_I_BUF("received magic", header.magic, sizeof(header.magic));
        errno = EPROTO;
        return -1;
    }

    // Check/detect version
    if (header.version != version_) {
        // Server not in version detection mode
        if (version_ != PROTOCOL_NO_VERSION) {
            LOG_E("Wrong version %d, expected %d", header.version, version_);
            errno = ECHRNG;
            return -1;
        }

        // Version does not match any known one
        if ((header.version != PROTOCOL_V2_VERSION) &&
                (header.version != PROTOCOL_V3_VERSION)) {
            LOG_E("Unsupported version %d", header.version);
            errno = ECHRNG;
            return -1;
        }

        LOG_I("Detected client protocol version %d", header.version);
        version_ = static_cast<ProtocolVersion>(header.version);
    }
    *method = static_cast<RpcMethod>(header.method);
    buffer->resize(header.length);
    LOG_D("%s: receive %u bytes of data", methodToString(*method), header.length);

    // Receive request data
    if (receive(*method, "data", &(*buffer)[0], header.length, signal_received)) {
        return -1;
    }

    return header.length;
}

int CommonConnection::sendResponse(
    RpcMethod method,
    const std::string& buffer,
    int proto_rc,
    int method_rc,
    bool* signal_received) {
    // Send response header
    ResponseHeader header;
    ::memcpy(header.magic, PROTOCOL_MAGIC, sizeof(header.magic));
    header.version = static_cast<uint16_t>(version_);
    header.reserved = 0;
    header.proto_rc = proto_rc;
    header.method_rc = method_rc;
    if (send(method, "header", &header, sizeof(header), signal_received)) {
        return -1;
    }

    // Send response data
    if (header.proto_rc > 0) {
        LOG_D("send %d bytes of data", header.proto_rc);
        if (send(method, "data", &buffer[0], header.proto_rc, signal_received)) {
            return -1;
        }
    }

    return 0;
}

int CommonConnection::receiveResponse(
    RpcMethod method,
    std::string* buffer,
    int* method_rc,
    bool* signal_received) {
    // Receive response header
    ResponseHeader header;
    if (receive(method, "header", &header, sizeof(header), signal_received)) {
        return -1;
    }

    // Check header
    if (::memcmp(header.magic, PROTOCOL_MAGIC, sizeof(header.magic))) {
        LOG_E("Wrong magic");
        errno = EPROTO;
        return -1;
    }
    if (header.version != version_) {
        LOG_E("Wrong version %d, expected %d", header.version, version_);
        errno = ECHRNG;
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
        if (receive(method, "data", &(*buffer)[0], length, signal_received)) {
            return -1;
        }
    }

    *method_rc = header.method_rc;
    return header.proto_rc;
}
