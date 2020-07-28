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

#ifndef __PROXY_V3_INTERNAL_H__
#define __PROXY_V3_INTERNAL_H__

#include "condition_variable"
#include "memory"
#include "mutex"
#include "vector"

#include "jni.h"
#include "proxy_client.h"
#include "gp_operations.h"
#include "gp_shared_mem.h"
#include "proxy.h"

#define PROXY_COPY_MAX_VERSION 0x00050000

namespace trustonic {

struct Proxy::Impl {
    Proxy&          parent;
    ProxyClient     client;
    TEECAllocatedSharedMemories gp_allocated_shared_memories;
    TEECOngoingOperations operations;
    // Versions up to 311 of MobiCore/<t-base/Kinibi auto-close the session on
    // TEEC_InvokeCommand error in some cases.
    bool            quirk_session_auto_close;
    JavaVM*         jvm = nullptr;
    jobject         application_context = nullptr;
    std::mutex      init_mutex;
    int             init_count = 0;

    // Keep track of shared memories in copy case
    class GpSharedMemoryTracker {
        TEEC_SharedMemory mem_;
    public:
        GpSharedMemoryTracker(const TEEC_SharedMemory& mem): mem_(mem) {}
        bool operator==(const TEEC_SharedMemory* other) {
            return (other->buffer == mem_.buffer) && (other->size == mem_.size) &&
                    (other->flags == mem_.flags);
        }
    };
    std::vector<GpSharedMemoryTracker> gp_shared_memories;
    std::mutex gp_shared_memories_mutex;

    Impl(Proxy& p): parent(p), quirk_session_auto_close(true) {}

    TEEC_Result serializeOperation(
            const TEEC_Operation* teec_operation,
            trustonic::GpOperation* proxy_operation);

    bool parseOperation(
            const trustonic::GpOperation& proxy_operation,
            TEEC_Operation* teec_operation);

    TEEC_Result TEEC_OpenSession(
            TEEC_Context*           context,
            TEEC_Session*           session,
            const TEEC_UUID*        destination,
            uint32_t                connectionMethod,
            const void*             connectionData,
            TEEC_Operation*         operation,
            uint32_t*               returnOrigin);

    TEEC_Result TEEC_InvokeCommand(
            TEEC_Session*           session,
            uint32_t                commandID,
            TEEC_Operation*         operation,
            uint32_t*               returnOrigin);
};

}

#endif // __PROXY_V3_INTERNAL_H__
