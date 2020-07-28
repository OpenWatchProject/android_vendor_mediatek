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

#include <algorithm>
#include <mutex>
#include <string>
#include <vector>

#include <unistd.h>     // sysconf
#include <stdlib.h>     // malloc, free
#include <sys/mman.h>   // mmap, munmap

#include "mc_user.h"    // mc_ioctl_buffer

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#include "mc.pb.h"
#pragma GCC diagnostic pop

#define SOCKET_PATH             "@/com/trustonic/tee_proxy"
#define PROTOCOL_MAGIC          "T7e3"
#define PROTOCOL_VERSION        2

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

namespace com {
namespace trustonic {
namespace tee_proxy {

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
    OPEN_SESSION = 0,
    OPEN_TRUSTLET = 1,
    CLOSE_SESSION = 2,
    NOTIFY = 3,
    WAIT = 4,
    MAP = 5,
    UNMAP = 6,
    GET_ERROR = 7,
    GET_VERSION = 9,
    GP_REQUESTCANCELLATION = 27,
    AUTHENTIFIER_SEND_LIST = 30,
};

class Session {
public:
    class Buffer {
        enum Type {
            NONE,       // No buffer
            CLIENT,     // Buffer managed by caller (client side)
            SERVER,     // Buffer mmap'd (server side)
        };
        mc_ioctl_buffer info_;
        void* address_;
        Type type_;
        int alloc(size_t length, uint32_t flags = MC_IO_MAP_INPUT_OUTPUT) {
            // No posix_memalign, aligned_alloc, valloc, memalign, pvalloc in
            // Android so we rely on mmap to give us page-aligned memory
            size_t page_mask = sysconf(_SC_PAGESIZE) - 1;
            size_t aligned_length = (length + page_mask) & ~page_mask;
            void* buf = ::mmap(0, aligned_length, PROT_READ | PROT_WRITE,
                               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (buf == MAP_FAILED) {
                LOG_E("Failed to allocate");
                return -1;
            }
            type_ = SERVER;
            address_ = buf;
            info_.va = reinterpret_cast<uintptr_t>(address_);
            info_.len = static_cast<uint32_t>(length);
            info_.flags = flags;
            LOG_D("alloc'd buffer %p:%u:%x", address_, info_.len, info_.flags);
            return 0;
        }
    public:
        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;
        // Constructor from buffer: allocate and fill in
        Buffer(std::string buffer): address_(nullptr) {
            if (!alloc(buffer.length())) {
                update(buffer);
            }
        }
        // Constructor for client TCI: pointer and length (can be null) given
        Buffer(void* address, size_t length): address_(address) {
            if (address_) {
                info_.va = reinterpret_cast<uintptr_t>(address_);
                info_.len = static_cast<uint32_t>(length);
                info_.flags = MC_IO_MAP_INPUT_OUTPUT;
                type_ = CLIENT;
                LOG_D("use buffer %p:%u:%x", address_, info_.len, info_.flags);
            } else {
                info_.va = 0;
                info_.len = 0;
                info_.flags = 0;
                type_ = NONE;
            }
            info_.sva = 0;
        }
        // Constructor for server buffer: allocate
        Buffer(uint32_t length, uint32_t flags): address_(nullptr) {
            alloc(length, flags);
        }
        // Constructor for client buffer: info given
        Buffer(mc_ioctl_buffer info): info_(info) {
            address_ = reinterpret_cast<void*>(
                           static_cast<uintptr_t>(info_.va));
            type_ = CLIENT;
            LOG_D("use buffer %p:%u:%x", address_, info_.len, info_.flags);
        }
        ~Buffer() {
            if (type_ == Buffer::SERVER) {
                LOG_D("unmap buffer %p:%u:%x", address_, info_.len,
                      info_.flags);
                ::munmap(address_, info_.len);
            }
        }
        // Accessors
        const mc_ioctl_buffer& info() const {
            return info_;
        }
        void* address() const {
            return address_;
        }
        void setSva(uint64_t sva) {
            info_.sva = sva;
        }
        int update(const std::string& buf) {
            if (buf.length() != info_.len) {
                LOG_E("Failed to update TCI");
                return -1;
            }
            if (type_ != NONE) {
                ::memcpy(address_, buf.c_str(), info_.len);
            }
            return 0;
        }
    };
private:
    uint32_t id_;
    Buffer* tci_;
    std::mutex buffers_mutex_;
    std::vector<Buffer*> buffers_;
public:
    Session(uint32_t id, Buffer* tci): id_(id), tci_(tci) {}
    ~Session() {
        delete tci_;
        for (auto& buf: buffers_) {
            delete buf;
        }
    }
    uint32_t id() const {
        return id_;
    }
    bool hasTci() const {
        return tci_;
    }
    const void* tci() const {
        return tci_->address();
    }
    size_t tciLen() const {
        return tci_->info().len;
    }
    int updateTci(const std::string& buf) {
        return tci_->update(buf);
    }
    void addBuffer(Buffer* buffer) {
        LOG_D("%p %s: 0x%llx", this, __FUNCTION__, buffer->info().sva);
        std::lock_guard<std::mutex> buffers_lock(buffers_mutex_);
        buffers_.push_back(buffer);
    }
    void addBuffer(mc_ioctl_buffer& info) {
        LOG_D("%p %s: 0x%llx", this, __FUNCTION__, info.sva);
        std::lock_guard<std::mutex> buffers_lock(buffers_mutex_);
        auto buffer = new Buffer(info);
        buffers_.push_back(buffer);
    }
    int removeBuffer(uint64_t sva) {
        LOG_D("%p %s: %jx", this, __FUNCTION__, sva);
        std::lock_guard<std::mutex> buffers_lock(buffers_mutex_);
        auto it = std::find_if(buffers_.begin(), buffers_.end(),
        [this, sva](Buffer* buffer) {
            return buffer->info().sva == sva;
        });
        if (it == buffers_.end()) {
            // Not found
            return -1;
        }
        delete *it;
        buffers_.erase(it);
        return 0;
    }
    // Must be called under buffers_mutex_
    Buffer* findBuffer(uint64_t sva) {
        for (auto& buf: buffers_) {
            if (buf->info().sva == sva) {
                return buf;
            }
        }
        return nullptr;
    }
    std::mutex& buffersMutex() {
        return buffers_mutex_;
    }
    const std::vector<Buffer*>& buffers() {
        return buffers_;
    }
};

class CommonConnection {
    int sock_;
    pid_t pid_ = 0;
    uid_t uid_ = -1;
    int send(
        const char* what,
        const void* buffer,
        size_t length,
        bool* signal_received);
    int receive(
        const char* what,
        void* buffer,
        size_t length,
        bool* signal_received,
        bool may_close = false);
public:
    CommonConnection(int sock = -1): sock_(sock) {}
    ~CommonConnection();
    int connect();
    int fd() const {
        return sock_;
    }
    pid_t pid() const {
        return pid_;
    }
    uid_t uid() const {
        return uid_;
    }
    int getClientCredentials();
    int sendRequest(
        enum RpcMethod method,
        const std::string& buffer,
        bool* signal_received = nullptr);
    int receiveRequest(
        enum RpcMethod* method,
        std::string* buffer,
        bool* signal_received = nullptr);
    int sendResponse(
        const std::string& buffer,
        int proto_rc,
        int method_rc,
        bool* signal_received = nullptr);
    int receiveResponse(
        std::string* buffer,
        int* method_rc,
        bool* signal_received = nullptr);
};

static const int32_t timeout_max = 1000;        // 1s

__attribute__((unused))
static const char* methodToString(enum RpcMethod method) {
    switch (method) {
        case OPEN_SESSION:
            return "openSession";
        case OPEN_TRUSTLET:
            return "openTruslet";
        case CLOSE_SESSION:
            return "closeSession";
        case NOTIFY:
            return "notify";
        case WAIT:
            return "waitNotification";
        case MAP:
            return "map";
        case UNMAP:
            return "unmap";
        case GET_ERROR:
            return "getError";
        case GET_VERSION:
            return "getVersion";
        case GP_REQUESTCANCELLATION:
            return "gpRequestCancellation";
        case AUTHENTIFIER_SEND_LIST:
            return "authentifierSendList";
    }
    return "unknown";
}

}  // namespace tee_proxy
}  // namespace trustonic
}  // namespace com
