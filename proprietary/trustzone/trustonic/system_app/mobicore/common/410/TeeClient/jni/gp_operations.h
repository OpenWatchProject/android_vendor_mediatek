/*
 * Copyright (c) 2016-2018 TRUSTONIC LIMITED
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

#ifndef __GP_OPERATIONS_H__
#define __GP_OPERATIONS_H__

#include <algorithm>
#include <mutex>
#include <vector>

#include "tee_client_api.h"

namespace trustonic {

// Keep internal track of operations, to properly deal with cancellation
class TEECOngoingOperations {
    static const uint32_t ENDED = 0xf;
    std::mutex operation_id_counter_mutex_;
    uint32_t operation_id_counter_ = 0x421;
    // List management
    std::mutex mutex_;
    std::vector<TEEC_Operation*> ongoing_operations_;
    std::vector<TEEC_Operation*> cancelled_operations_;
    // Manage a 28-bit counter for operation IDs
    uint32_t getOperationId() {
        std::lock_guard<std::mutex> lock(operation_id_counter_mutex_);
        operation_id_counter_++;
        if (!(operation_id_counter_ & 0xfffffff)) {
            operation_id_counter_ = 1;
        }
        return operation_id_counter_;
    }

public:
    bool start(TEEC_Operation* operation) {
        LOG_D("operation %p start", operation);
        std::lock_guard<std::mutex> lock(mutex_);
        // Check for early cancellation
        auto it = std::find(cancelled_operations_.begin(), cancelled_operations_.end(),
                            operation);
        if (it != cancelled_operations_.end()) {
            LOG_I("operation cancelled before start");
            cancelled_operations_.erase(it);
            return false;
        }

        auto operation_id = getOperationId();
        // Leave space for status bits below operation ID
        operation->started = operation_id << 4;
        LOG_D("operation %p (ID %u) started", operation, operation_id);
        ongoing_operations_.push_back(operation);
        return true;
    }

    void end(TEEC_Operation* operation) {
        LOG_D("operation %p end", operation);
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = std::find(ongoing_operations_.begin(), ongoing_operations_.end(),
                            operation);
        if (it == ongoing_operations_.end()) {
            LOG_E("operation %p not ongoing, cannot end", operation);
            return;
        }
        ongoing_operations_.erase(it);
        operation->started = ENDED;
        LOG_D("operation %p ended", operation);
    }

    bool cancel(TEEC_Operation* operation) {
        LOG_D("operation %p cancel", operation);
        std::lock_guard<std::mutex> lock(mutex_);
        // Look in list first in case started field was reset by client
        auto it = std::find(ongoing_operations_.begin(), ongoing_operations_.end(),
                            operation);
        if (it != ongoing_operations_.end()) {
            LOG_D("operation %p ongoing, request cancellation", operation);
            return true;
        }

        if (operation->started == 0) {
            // Operation not started yet, add to cancelled
            auto it = std::find(cancelled_operations_.begin(), cancelled_operations_.end(),
                                operation);
            if (it == cancelled_operations_.end()) {
                cancelled_operations_.push_back(operation);
                LOG_D("operation %p cancellation recorded", operation);
            } else {
                LOG_D("operation %p cancellation was already recorded", operation);
            }
        } else {
            LOG_I("operation ended before cancellation");
        }

        return false;
    }
};

}

#endif // __GP_OPERATIONS_H__

