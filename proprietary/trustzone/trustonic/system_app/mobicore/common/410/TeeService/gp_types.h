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

#ifndef GP_TYPES_H
#define GP_TYPES_H

#include <string.h>

#include <algorithm>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

#include <tee_client_api.h>
#include "common_types.h"

namespace vendor {
namespace trustonic {

class GpContext;

class GpSession: public Session {
    std::shared_ptr<GpContext> context_;
    uint64_t reference_ = 0;
    TEEC_Session gp_session_;
    std::mutex operation_mutex_;
    std::condition_variable operation_cond_;
    TEEC_Operation* operation_ = nullptr;
    uint64_t operation_reference_ = 0;
    bool operation_set_ = false;
public:
    GpSession(
            const std::shared_ptr<Service>& service,
            const std::shared_ptr<GpContext>& context,
            uint64_t reference):
        Session(service), context_(context), reference_(reference) {
        ::memset(&gp_session_, 0, sizeof(gp_session_));
    }

    std::shared_ptr<GpContext> context() const {
        return context_;
    }

    uint64_t reference() const {
        return reference_;
    }

    TEEC_Session& gp() {
        return gp_session_;
    }

    void resetReference() {
        ALOGH("reference reset for session %ju", reference_);
        reference_ = 0xffffffffffffffff;
    }

    void setOperation(
            uint64_t reference,
            TEEC_Operation* operation) {
        std::unique_lock<std::mutex> lock(operation_mutex_);
        while (operation_set_) {
            operation_cond_.wait(lock);
        }

        operation_ = operation;
        operation_reference_ = reference;
        operation_set_ = true;
    }

    void resetOperation() {
        std::lock_guard<std::mutex> lock(operation_mutex_);
        operation_ = nullptr;
        operation_reference_ = 0;
        operation_set_ = false;
        operation_cond_.notify_one();
    }

    TEEC_Operation* operation(
            uint64_t reference) {
        std::lock_guard<std::mutex> lock(operation_mutex_);
        ALOGH("current operation %jx, wanted %jx", operation_reference_, reference);
        if (reference == operation_reference_) {
            return operation_;
        }

        return nullptr;
    }
};

class GpContext: public Client {
    std::mutex sessions_mutex_;
    std::vector<std::shared_ptr<GpSession>> sessions_;
    uint64_t reference_ = 0;
    TEEC_Context gp_context_;

    auto findSessionUnlocked(
            uint64_t reference) {
        return std::find_if(sessions_.begin(), sessions_.end(), [reference]
                            (const auto& session) {
            return session->reference() == reference;
        }
        );
    }
public:
    GpContext(
            const std::shared_ptr<Service>& service,
            uint64_t context = 0):
        Client(service), reference_(context) {
        ::memset(&gp_context_, 0, sizeof(gp_context_));
    }

    uint64_t reference() const {
        return reference_;
    }

    TEEC_Context& gp() {
        return gp_context_;
    }

    void addSession(
            const std::shared_ptr<GpSession>& session) {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        // Look for an entry with same reference, in which case nuke previous
        // reference (older session becomes inaccessible).
        auto it = findSessionUnlocked(session->reference());
        if (it != sessions_.end()) {
            auto& lost_session = *it;
            lost_session->resetReference();
        }

        sessions_.emplace_back(session);
        ALOGM("%zu sessions for context %jx", sessions_.size(), reference_);
    }

    bool removeSession(
            uint64_t reference) {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        auto it = findSessionUnlocked(reference);
        if (it == sessions_.end()) {
            return false;
        }

        sessions_.erase(it);
        ALOGM("%zu sessions for context %jx", sessions_.size(), reference_);
        return true;
    }

    std::shared_ptr<GpSession> getSession(
            uint64_t reference) {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        auto it = findSessionUnlocked(reference);
        if (it == sessions_.end()) {
            return nullptr;
        }

        return *it;
    }

    std::shared_ptr<GpSession> getSessionByOperation(
            uint64_t reference) {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        ALOGH("Find operation %jx in %zu sessions", reference, sessions_.size());
        auto it = std::find_if(sessions_.begin(), sessions_.end(), [reference]
                               (const auto& session) {
            return session->operation(reference);
        }
        );
        if (it == sessions_.end()) {
            return nullptr;
        }

        return *it;
    }

    void flush() override {
        Client::flush();
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        for (auto& session: sessions_) {
            session->flush();
        }

        sessions_.clear();
        ALOGM("%zu sessions", sessions_.size());
    }
};

class GpClient {
    std::mutex contexts_mutex_;
    std::vector<std::shared_ptr<GpContext>> contexts_;
    std::mutex cancelled_operations_mutex_;
    std::vector<uint64_t> cancelled_operations_;
    int pid_ = 0;

    auto findContextUnlocked(
            uint64_t reference) {
        return std::find_if(contexts_.begin(), contexts_.end(), [reference]
                            (const auto& gp_context) {
            return gp_context->reference() == reference;
        }
        );
    }
public:
    GpClient(
            int pid = 0):
        pid_(pid) {
        ALOGM("%s", __func__);
    }

    ~GpClient() {
        ALOGM("%s", __func__);
    }

    int pid() const {
        return pid_;
    }

    void addContext(
            std::shared_ptr<GpContext> gp_context) {
        std::lock_guard<std::mutex> lock(contexts_mutex_);
        contexts_.emplace_back(gp_context);
        ALOGM("%zu contexts for PID %d", contexts_.size(), pid_);
    }

    bool removeContext(
            uint64_t reference) {
        std::lock_guard<std::mutex> lock(contexts_mutex_);
        auto it = findContextUnlocked(reference);
        if (it != contexts_.end()) {
            auto& context = *it;
            context->flush();
            contexts_.erase(it);
            ALOGM("%zu contexts for PID %d", contexts_.size(), pid_);
            return true;
        }

        return false;
    }

    bool hasNoContexts() {
        std::lock_guard<std::mutex> lock(contexts_mutex_);
        return contexts_.empty();
    }

    std::shared_ptr<GpContext> getContext(
            uint64_t reference) {
        std::lock_guard<std::mutex> lock(contexts_mutex_);
        auto it = findContextUnlocked(reference);
        if (it == contexts_.end()) {
            return nullptr;
        }

        return *it;
    }

    std::shared_ptr<IBuffer> getBuffer(
            void* buf) {
        std::shared_ptr<IBuffer> buffer;
        std::lock_guard<std::mutex> lock(contexts_mutex_);
        for (auto& context: contexts_) {
            buffer = context->getBuffer(buf);
            if (buffer) {
                break;
            }
        }

        if (!buffer) {
            ALOGE("Failed to find requested buffer.");
            return nullptr;
        }

        return buffer;
    }

    std::shared_ptr<GpSession> getSession(
            uint64_t reference) {
        std::shared_ptr<GpSession> session;
        std::lock_guard<std::mutex> lock(contexts_mutex_);
        for (auto& context: contexts_) {
            session = context->getSession(reference);
            if (session) {
                break;
            }
        }

        if (!session) {
            return nullptr;
        }

        return session;
    }

    std::shared_ptr<GpSession> getSessionByOperation(
            uint64_t reference) {
        std::lock_guard<std::mutex> lock(contexts_mutex_);
        ALOGH("Find operation %jx in %zu contexts", reference, contexts_.size());
        for (auto& context: contexts_) {
            auto session = context->getSessionByOperation(reference);
            if (session) {
                return session;
            }
        }

        return nullptr;
    }

    void addCancelledOperation(
            uint64_t reference) {
        std::lock_guard<std::mutex> lock(cancelled_operations_mutex_);
        cancelled_operations_.push_back(reference);
        ALOGH("add cancelled operation %jx", reference);
        ALOGM("%zu cancelled operations", cancelled_operations_.size());
    }

    bool isCancelledOperation(
            uint64_t reference) {
        std::lock_guard<std::mutex> lock(cancelled_operations_mutex_);
        ALOGH("find cancelled operation %jx", reference);
        auto it = std::find(cancelled_operations_.begin(),
                            cancelled_operations_.end(), reference);
        if (it == cancelled_operations_.end()) {
            return false;
        }

        cancelled_operations_.erase(it);
        ALOGH("remove cancelled operation %jx", reference);
        ALOGM("%zu cancelled operations", cancelled_operations_.size());
        return true;
    }

    void flush() {
        std::lock_guard<std::mutex> lock(contexts_mutex_);
        for (auto& context: contexts_) {
            context->flush();
        }

        contexts_.clear();
        ALOGM("%zu contexts", contexts_.size());
    }
};

class GpManager {
    std::mutex clients_mutex_;
    std::vector<std::shared_ptr<GpClient>> clients_;

    auto findClientUnlocked(
            int pid) {
        return std::find_if(clients_.begin(), clients_.end(), [pid]
                            (const auto& client) {
            return client->pid() == pid;
        }
        );
    }
public:
    void addContext(
            int pid,
            std::shared_ptr<GpContext> gp_context) {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        std::shared_ptr<GpClient> client;
        auto it = findClientUnlocked(pid);
        if (it == clients_.end()) {
            client = std::make_shared<GpClient>(pid);
            clients_.emplace_back(client);
            ALOGM("%zu clients", clients_.size());
        } else {
            client = *it;
        }

        client->addContext(gp_context);
    }

    std::shared_ptr<GpContext> createContext(
            const std::shared_ptr<Service>& service,
            int pid,
            uint64_t reference) {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        std::shared_ptr<GpClient> client;
        auto it = findClientUnlocked(pid);
        if (it == clients_.end()) {
            client = std::make_shared<GpClient>(pid);
            clients_.emplace_back(client);
            ALOGM("%zu clients", clients_.size());
        } else {
            client = *it;
        }

        auto gp_context = std::make_shared<GpContext>(service, reference);
        client->addContext(gp_context);
        return gp_context;
    }

    bool removeContext(
            int pid,
            uint64_t reference) {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        auto it = findClientUnlocked(pid);
        if (it == clients_.end()) {
            return false;
        }

        auto client = *it;
        if (!client->removeContext(reference)) {
            return false;
        }

        if (client->hasNoContexts()) {
            clients_.erase(it);
            ALOGM("%zu clients", clients_.size());
        }

        return true;
    }

    std::shared_ptr<GpContext> getContext(
            int pid,
            uint64_t reference) {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        auto it = findClientUnlocked(pid);
        if (it == clients_.end()) {
            return nullptr;
        }

        auto client = *it;
        return client->getContext(reference);
    }

    std::shared_ptr<GpClient> getClient(
            int pid) {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        auto it = findClientUnlocked(pid);
        if (it == clients_.end()) {
            return nullptr;
        }

        return *it;
    }

    void flush() {
        ALOGH("%s", __PRETTY_FUNCTION__);
        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (auto& client: clients_) {
            client->flush();
        }

        clients_.clear();
        ALOGM("%zu clients", clients_.size());
    }

    void flushPid(int pid) {
        ALOGH("%s pid %d", __PRETTY_FUNCTION__, pid);
        std::lock_guard<std::mutex> lock(clients_mutex_);
        auto it = findClientUnlocked(pid);
        if (it == clients_.end()) {
            return;
        }

        auto client = *it;
        client->flush();
        clients_.erase(it);
        ALOGM("%zu clients", clients_.size());
    }
};

}
}

#endif // GP_TYPES_H
