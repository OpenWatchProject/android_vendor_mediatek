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

#ifndef __PROXY_COMMON_H__
#define __PROXY_COMMON_H__

#include <memory>
#include <string>

#include <unistd.h>     // sysconf
#include <sys/mman.h>   // mmap, munmap

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#include <proxy.pb.h>
#pragma GCC diagnostic pop

#define SOCKET_TDP_PATH         "@/com/trustonic/tee_proxy_service"
#define SOCKET_TUP_PATH         "@/com/trustonic/tee_proxy"

namespace trustonic {

enum ProtocolVersion {
    PROTOCOL_NO_VERSION = 1,    // Version to be determined
    PROTOCOL_V2_VERSION = 2,    // TDP 1.0 (copy)
    PROTOCOL_V3_VERSION = 3,    // TDP 1.1+ (copy)
};

#define PROTOCOL_MAGIC          "T7e3"

#define ZEROCOPY_MIN_VERSION    0x00060003

/*
 * ProtoBuf gives us the serialisation mechanism, but that's not enough to send
 * our RPC messages: we also need to pass the method we want to call, the total
 * length of the data, and a magic number is usually welcome too. While at it,
 * we'll throw a version number just in case.
 *
 * Hence:
 *               ----------------------
 *              |     Magic number     |        4 bytes (text)
 *               ----------------------
 *              |  Method  |  Version  |        2 + 2 bytes (LE)
 *               ----------------------
 *              |    Message length    |        4 bytes (LE)
 *               ----------------------
 *              |                      |
 *              |     Message data     |        N bytes (text)
 *              ~                      ~
 *              |                      |
 *               ----------------------
 */

struct RequestHeader {
    char magic[4];
    uint16_t version;
    uint16_t method;
    uint32_t length;
};

struct ResponseHeader {
    char magic[4];
    uint16_t version;
    uint16_t reserved;
    int32_t proto_rc;           // -errno if negative, length of data otherwise
    uint32_t method_rc;         // errno from called method on server side
};

enum RpcMethod {
    UNKNOWN = -1,               // When receiving requests on the server side
    OPEN_SESSION = 0,
    OPEN_TRUSTLET = 1,
    CLOSE_SESSION = 2,
    NOTIFY = 3,
    WAIT = 4,
    MAP = 5,
    UNMAP = 6,
    GET_ERROR = 7,
    GET_VERSION = 9,
    GP_INITIALIZE_CONTEXT = 20,
    GP_REGISTER_SHARED_MEM = 21,
    GP_RELEASE_SHARED_MEM = 23,
    GP_OPEN_SESSION = 24,
    GP_CLOSE_SESSION = 25,
    GP_INVOKE_COMMAND = 26,
    GP_REQUEST_CANCELLATION = 27,
};

class CommonConnection {
    std::string path_;
    ProtocolVersion version_;
    int sock_ = -1;
    bool is_server_ = false;
    pid_t pid_ = 0;
    uid_t uid_ = -1;
    int send(
        RpcMethod method,
        const char* what,
        const void* buffer,
        size_t length,
        bool* signal_received);
    int receive(
        RpcMethod method,
        const char* what,
        void* buffer,
        size_t length,
        bool* signal_received,
        bool may_close = false);
    CommonConnection(int sock, ProtocolVersion version):
        version_(version), sock_(sock) {}
    int getClientCredentials();
    int sendCredentials();
    int receiveCredentials();
public:
    CommonConnection(const std::string& path,
                     ProtocolVersion version = PROTOCOL_NO_VERSION):
        path_(path), version_(version) {}
    ~CommonConnection();
    ProtocolVersion version() const {
        return version_;
    }
    // Server
    int listen();
    // Connection
    std::unique_ptr<CommonConnection> accept();
    int connect();
    void close();
    int fd() const {
        return sock_;
    }
    pid_t pid() const {
        return pid_;
    }
    uid_t uid() const {
        return uid_;
    }
    int sendRequest(
        RpcMethod method,
        const std::string& buffer,
        bool* signal_received = nullptr);
    int receiveRequest(
        RpcMethod* method,
        std::string* buffer,
        bool* signal_received = nullptr);
    int sendResponse(
        RpcMethod method,
        const std::string& buffer,
        int proto_rc,
        int method_rc,
        bool* signal_received = nullptr);
    int receiveResponse(
        RpcMethod method,
        std::string* buffer,
        int* method_rc,
        bool* signal_received = nullptr);
};

static const int32_t timeout_max = 1000;        // 1s

__attribute__((unused))
static const char* methodToString(RpcMethod method) {
    switch (method) {
        case UNKNOWN:
            break;
        case OPEN_SESSION:
            return "open session";
        case OPEN_TRUSTLET:
            return "open truslet";
        case CLOSE_SESSION:
            return "close session";
        case NOTIFY:
            return "notify";
        case WAIT:
            return "wait notification";
        case MAP:
            return "map";
        case UNMAP:
            return "unmap";
        case GET_ERROR:
            return "get error";
        case GET_VERSION:
            return "get version";
        case GP_INITIALIZE_CONTEXT:
            return "GP initialize context";
        case GP_REGISTER_SHARED_MEM:
            return "GP register shared mem";
        case GP_RELEASE_SHARED_MEM:
            return "GP release shared mem";
        case GP_OPEN_SESSION:
            return "GP open session";
        case GP_CLOSE_SESSION:
            return "GP close session";
        case GP_INVOKE_COMMAND:
            return "GP invoke command";
        case GP_REQUEST_CANCELLATION:
            return "GP request cancellation";
    }
    return "unknown";
}

}  // namespace trustonic

#endif // __PROXY_COMMON_H__
