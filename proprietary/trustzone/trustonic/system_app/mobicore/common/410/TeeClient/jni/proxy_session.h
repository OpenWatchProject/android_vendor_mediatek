/*
 * Copyright (c) 2013-2017 TRUSTONIC LIMITED
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

#ifndef __PROXY_SESSION_H__
#define __PROXY_SESSION_H__

#include <algorithm>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <sys/mman.h>

namespace trustonic {

class Session {
public:
    class Buffer {
        enum Type {
            NONE,       // No buffer
            CLIENT,     // Buffer managed by caller (client side)
            SERVER,     // Buffer mmap'd (server side)
        };
        uint64_t va_ = 0;
        uint32_t len_ = 0;
        uint64_t sva_ = 0;
        void* address_;
        Type type_;
        int alloc(size_t length) {
            // No posix_memalign, aligned_alloc, valloc, memalign, pvalloc in
            // Android so we rely on mmap to give us page-aligned memory
            size_t page_mask = sysconf(_SC_PAGESIZE) - 1;
            size_t aligned_length = (length + page_mask) & ~page_mask;
            void* buf = ::mmap(0, aligned_length, PROT_READ | PROT_WRITE,
                               MAP_SHARED | MAP_ANONYMOUS, -1, 0);
            if (buf == MAP_FAILED) {
                LOG_E("Failed to allocate");
                return -1;
            }
            type_ = SERVER;
            address_ = buf;
            va_ = reinterpret_cast<uintptr_t>(address_);
            len_ = static_cast<uint32_t>(length);
            LOG_D("alloc'd buffer %p:%u", address_, len_);
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
                va_ = reinterpret_cast<uintptr_t>(address_);
                len_ = static_cast<uint32_t>(length);
                type_ = CLIENT;
                LOG_D("use buffer %p:%u", address_, len_);
            } else {
                type_ = NONE;
            }
        }
        // Constructor for server buffer: allocate
        Buffer(uint32_t length): address_(nullptr) {
            alloc(length);
        }
        // Constructor for client buffer: info given
        Buffer(uint64_t va, uint32_t len, uint64_t sva):
                va_(va), len_(len), sva_(sva) {
            address_ = reinterpret_cast<void*>(
                           static_cast<uintptr_t>(va_));
            type_ = CLIENT;
            LOG_D("use buffer %p:%u", address_, len_);
        }
        ~Buffer() {
            if (type_ == Buffer::SERVER) {
                LOG_D("unmap buffer %p:%u", address_, len_);
                ::munmap(address_, len_);
            }
        }
        // Accessors
        uint64_t va() const {
            return va_;
        }
        uint32_t length() const {
            return len_;
        }
        uint64_t sva() const {
            return sva_;
        }
        void* address() const {
            return address_;
        }
        void setSva(uint64_t sva) {
            sva_ = sva;
        }
        int update(const std::string& buf) {
            if (buf.length() != len_) {
                LOG_E("Failed to update TCI");
                return -1;
            }
            if (type_ != NONE) {
                ::memcpy(address_, buf.c_str(), len_);
            }
            return 0;
        }
    };
private:
    uint32_t id_;
    std::shared_ptr<Buffer> tci_;
    std::mutex buffers_mutex_;
    std::vector<std::shared_ptr<Buffer>> buffers_;
public:
    Session(uint32_t id, const std::shared_ptr<Buffer>& tci): id_(id), tci_(tci) {}
    uint32_t id() const {
        return id_;
    }
    bool hasTci() const {
        return tci_.get();
    }
    const void* tci() const {
        return tci_->address();
    }
    size_t tciLen() const {
        return tci_->length();
    }
    int updateTci(const std::string& buf) {
        return tci_->update(buf);
    }
    void addBuffer(const std::shared_ptr<Buffer>& buffer) {
        LOG_D("%p %s: 0x%jx", this, __FUNCTION__, buffer->sva());
        std::lock_guard<std::mutex> buffers_lock(buffers_mutex_);
        buffers_.push_back(buffer);
    }
    void addBuffer(uint64_t va, uint32_t len, uint64_t sva) {
        LOG_D("%p %s: 0x%jx", this, __FUNCTION__, sva);
        std::lock_guard<std::mutex> buffers_lock(buffers_mutex_);
        auto buffer = std::make_shared<Buffer>(va, len, sva);
        buffers_.push_back(buffer);
    }
    int removeBuffer(uint64_t sva) {
        LOG_D("%p %s: %jx", this, __FUNCTION__, sva);
        std::lock_guard<std::mutex> buffers_lock(buffers_mutex_);
        auto it = std::find_if(buffers_.begin(), buffers_.end(),
            [sva](const std::shared_ptr<Buffer>& buffer) {
                return buffer->sva() == sva;
            });
        if (it == buffers_.end()) {
            // Not found
            return -1;
        }
        buffers_.erase(it);
        return 0;
    }
    std::shared_ptr<Buffer> findBuffer(uint64_t sva) {
        std::lock_guard<std::mutex> buffers_lock(buffers_mutex_);
        for (auto& buf: buffers_) {
            if (buf->sva() == sva) {
                return buf;
            }
        }
        return nullptr;
    }
    const std::vector<std::shared_ptr<Buffer>> buffers() const {
        std::vector<std::shared_ptr<Buffer>> bufs;
        for (auto& buf: buffers_) {
            bufs.push_back(buf);
        }
        return bufs;
    }
};

}  // namespace trustonic

#endif // __PROXY_SESSION_H__
