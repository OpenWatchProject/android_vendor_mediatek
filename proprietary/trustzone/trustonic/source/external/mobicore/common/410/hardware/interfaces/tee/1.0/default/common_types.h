/*
 * Copyright (c) 2018 TRUSTONIC LIMITED
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

#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <memory>
#include <mutex>
#include <vector>

#undef ALOGH
#ifdef TEESERVICE_DEBUG
#define ALOGH ALOGI
#else
#define ALOGH(...) do {} while(0)
#endif

#undef ALOGM
#ifdef TEESERVICE_MEMLEAK_CHECK
#include <malloc.h>
#define ALOGM(fmt, ...) \
    ALOGI(fmt " (MEMLEAK_CHECK %zu) [%s:%d:%s]", ##__VA_ARGS__, \
          static_cast<size_t>(mallinfo().uordblks), \
          __FILE__, __LINE__, __func__)
#else
#define ALOGM(...) do {} while(0)
#endif

namespace vendor {
namespace trustonic {

// To be implemented by each user of these classes
class Service;

// GP-specific
class GpContext;

class IBuffer {
    // Buffer unique reference in all instances
    // * MC: the SVA
    // * GP: the buffer address
    uint64_t reference_ = 0;
    // GP-specific
    std::shared_ptr<GpContext> context_;
    // Also used by MC, to copy only when needed
    uint32_t flags_ = TEEC_MEM_INOUT;
public:
    // Constructor for MC protocol
    IBuffer() {
        ALOGM("buffer");
    }

    // Constructor for GP protocol
    IBuffer(
            const std::shared_ptr<GpContext>& context,
            uint32_t flags):
        context_(context), flags_(flags) {
        ALOGM("%s", __func__);
    }

    virtual ~IBuffer() {
        ALOGM("%s", __func__);
    }

    void setReference(uint64_t reference) {
        reference_ = reference;
    }

    uint64_t reference() const {
        return reference_;
    }

    virtual std::shared_ptr<GpContext> context() const {
        return context_;
    }

    uint32_t flags() const {
        return flags_;
    }

    virtual int fd() const = 0;

    virtual void* address() const = 0;

    virtual uint32_t size() const = 0;

    virtual void updateDestination() = 0;

    virtual void updateSource() = 0;
};

class Session {
    std::mutex buffers_mutex_;
    std::vector<std::shared_ptr<IBuffer>> buffers_;
    std::shared_ptr<Service> service_;
public:
    Session(
            const std::shared_ptr<Service>& service):
        service_(service) {
        ALOGM("%s", __func__);
    }

    virtual ~Session() {
        ALOGM("%s", __func__);
    }

    std::shared_ptr<Service> getService() const {
        return service_;
    }

    void addBuffer(
            const std::shared_ptr<IBuffer>& buffer) {
        std::lock_guard<std::mutex> lock(buffers_mutex_);
        buffers_.emplace_back(buffer);
        ALOGM("%zu buffers", buffers_.size());
    }

    bool removeBuffer(
            std::shared_ptr<IBuffer> buffer_in) {
        std::lock_guard<std::mutex> lock(buffers_mutex_);
        auto it = std::find_if(buffers_.begin(), buffers_.end(), [buffer_in]
                               (const auto& buffer) {
            return buffer == buffer_in;
        }
        );
        if (it == buffers_.end()) {
            return false;
        }

        buffers_.erase(it);
        ALOGM("%zu buffers", buffers_.size());
        return true;
    }

    std::shared_ptr<IBuffer> getBuffer(
            uint64_t reference) {
        std::lock_guard<std::mutex> lock(buffers_mutex_);
        auto it = std::find_if(buffers_.begin(), buffers_.end(), [reference]
                               (const auto& buffer) {
            return buffer->reference() == reference;
        }
        );
        if (it == buffers_.end()) {
            return nullptr;
        }

        return *it;
    }

    void updateDestination() {
        for (auto& buffer: buffers_) {
            buffer->updateDestination();
        }
    }

    void updateSource() {
        for (auto& buffer: buffers_) {
            buffer->updateSource();
        }
    }

    virtual void flush() {
        std::lock_guard<std::mutex> lock(buffers_mutex_);
        buffers_.clear();
        ALOGM("%s %zu buffers", __func__, buffers_.size());
    }
};

class Client {
    std::mutex buffers_mutex_;
    std::vector<std::shared_ptr<IBuffer>> buffers_;
    std::shared_ptr<Service> service_;
public:
    Client(
            const std::shared_ptr<Service>& service):
        service_(service) {
        ALOGM("%s", __func__);
    }

    virtual ~Client() {
        ALOGM("%s", __func__);
    }

    std::shared_ptr<Service> getService() const {
        return service_;
    }

    void addBuffer(
            const std::shared_ptr<IBuffer>& buffer) {
        std::lock_guard<std::mutex> lock(buffers_mutex_);
        buffers_.emplace_back(buffer);
        ALOGM("%zu buffers", buffers_.size());
    }

    bool removeBuffer(
            std::shared_ptr<IBuffer> buffer_in) {
        std::lock_guard<std::mutex> lock(buffers_mutex_);
        auto it = std::find_if(buffers_.begin(), buffers_.end(), [buffer_in]
                               (const auto& buffer) {
            return buffer == buffer_in;
        }
        );
        if (it == buffers_.end()) {
            return false;
        }

        buffers_.erase(it);
        ALOGM("%zu buffers", buffers_.size());
        return true;
    }

    std::shared_ptr<IBuffer> getBuffer(
            void* buf) {
        std::lock_guard<std::mutex> lock(buffers_mutex_);
        auto it = std::find_if(buffers_.begin(), buffers_.end(), [buf]
                               (const auto& buffer) {
            return buffer->address() == buf;
        }
        );
        if (it == buffers_.end()) {
            return nullptr;
        }

        return *it;
    }

    std::shared_ptr<IBuffer> getBuffer(
            uint64_t reference) {
        std::lock_guard<std::mutex> lock(buffers_mutex_);
        auto it = std::find_if(buffers_.begin(), buffers_.end(), [reference]
                               (const auto& buffer) {
            return buffer->reference() == reference;
        }
        );
        if (it == buffers_.end()) {
            return nullptr;
        }

        return *it;
    }

    virtual void flush() {
        std::lock_guard<std::mutex> lock(buffers_mutex_);
        buffers_.clear();
        ALOGM("%s %zu buffers", __func__, buffers_.size());
    }
};

}
}

#endif // COMMON_TYPES_H

