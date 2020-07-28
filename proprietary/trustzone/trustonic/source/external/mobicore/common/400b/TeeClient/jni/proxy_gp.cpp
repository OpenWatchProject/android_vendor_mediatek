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

// C++
#include <algorithm>
#include <memory>
#include <vector>
// C
#include <stdlib.h>
// TEE
#include "mcinq.h"      /* TA termination codes */
// Log
#include "dynamic_log.h"
// API
#include "tee_client_api.h"
// Class
#include "proxy_internal.h"

/**
 * These error codes are still to be decided by GP and as we do not wish to
 * expose any part of the GP TAF as of yet, for now they will have to live here
 * until we decide what to do about them.
 */
#define TEEC_ERROR_TA_LOCKED        0xFFFF0257
#define TEEC_ERROR_SD_BLOCKED       0xFFFF0258
#define TEEC_ERROR_TARGET_KILLED    0xFFFF0259

// Macros
#define GET_PARAM_TYPE(t, i)    (((t) >> (4*i)) & 0xF)
#define PARAMS_NUMBER           4

using namespace trustonic;

class GpCaller: public ProxyClient::ICaller {
    GpOpenSessionResponse* response_ = nullptr;
    uint32_t session_id_ = 0;
    bool is_close_session_ = false;
    bool is_device_operation_ = false;
public:
    GpCaller(): is_device_operation_(true) {}
    GpCaller(uint32_t session_id, bool is_close_session = false):
        session_id_(session_id), is_close_session_(is_close_session) {}
    GpCaller(GpOpenSessionResponse* response): response_(response) {}
    bool isClientOperation() const override {
        return is_device_operation_;
    }
    bool isOpenSession() const override {
        return response_;
    }
    bool isCloseSession() const override {
        return is_close_session_;
    }
    uint32_t sessionId() const override {
        if (session_id_ || !response_) {
            return session_id_;
        }
        return response_->session_id();
    }
    void setSignalled() override {}
    bool isSignalled() const override {
        return false;
    }
};

TEEC_Result Proxy::Impl::serializeOperation(
    const TEEC_Operation* teec_operation,
    GpOperation* proxy_operation) {
    ENTER();
    // Initialise
    proxy_operation->set_param_types(0);
    if (!teec_operation) {
        EXIT(TEEC_SUCCESS);
    }

    TEEC_Result teec_result = TEEC_SUCCESS;
    proxy_operation->set_started(teec_operation->started);
    proxy_operation->set_param_types(teec_operation->paramTypes);
    LOG_D("param types %04x", teec_operation->paramTypes);
    for (size_t param_no = 0; (param_no < PARAMS_NUMBER) &&
            (teec_result == TEEC_SUCCESS); param_no++) {
        const TEEC_Parameter* teec_param = &teec_operation->params[param_no];
        uint32_t param_type = GET_PARAM_TYPE(teec_operation->paramTypes, param_no);

        switch (param_type) {
            case TEEC_NONE:
                LOG_D("  param #%zu, TEEC_NONE", param_no);
                break;
            case TEEC_VALUE_OUTPUT:
                LOG_D("  param #%zu, TEEC_VALUE_OUTPUT", param_no);
                break;
            case TEEC_VALUE_INPUT:
            case TEEC_VALUE_INOUT: {
                LOG_D("  param #%zu, TEEC_VALUE_IN*", param_no);
                GpParam* param = proxy_operation->add_params();
                GpValue* value = param->mutable_value();
                value->set_a(teec_param->value.a);
                value->set_b(teec_param->value.b);
                break;
            }
            case TEEC_MEMREF_TEMP_OUTPUT:
            case TEEC_MEMREF_TEMP_INPUT:
            case TEEC_MEMREF_TEMP_INOUT: {
                // A Temporary Memory Reference may be null, which can be used
                // to denote a special case for the parameter. Output Memory
                // References that are null are typically used to request the
                // required output size.
                LOG_D("  param #%zu, TEEC_MEMREF_TEMP_* %p/%zu", param_no,
                      teec_param->tmpref.buffer, teec_param->tmpref.size);
                GpParam* param = proxy_operation->add_params();
                GpTempMemref* tmpref = param->mutable_tmpref();
                tmpref->set_buffer(reinterpret_cast<uintptr_t>(teec_param->tmpref.buffer));
                tmpref->set_size(teec_param->tmpref.size);
                if ((param_type != TEEC_MEMREF_TEMP_OUTPUT) &&
                        teec_param->tmpref.buffer) {
                    LOG_D("set contents");
                    tmpref->set_contents(teec_param->tmpref.buffer, teec_param->tmpref.size);
                }
                LOG_D("buffer %p, size %zu", teec_param->tmpref.buffer,
                      teec_param->tmpref.size);
                break;
            }
            case TEEC_MEMREF_WHOLE: {
                LOG_D("  param #%zu, TEEC_MEMREF_WHOLE %p/%zu/%x", param_no,
                      teec_param->memref.parent->buffer, teec_param->memref.parent->size,
                      teec_param->memref.parent->flags);
                GpParam* param = proxy_operation->add_params();
                GpRegisteredMemref* memref = param->mutable_memref();
                GpSharedMemory* parent = memref->mutable_parent();
                parent->set_buffer(reinterpret_cast<uintptr_t>
                                   (teec_param->memref.parent->buffer));
                parent->set_size(teec_param->memref.parent->size);
                parent->set_flags(teec_param->memref.parent->flags & TEEC_MEM_INOUT);
                std::lock_guard<std::mutex> lock(gp_shared_memories_mutex);
                auto it = std::find(gp_shared_memories.begin(), gp_shared_memories.end(),
                                    teec_param->memref.parent);
                if (it == gp_shared_memories.end()) {
                    LOG_E("shared memory not registered");
                    teec_result = TEEC_ERROR_BAD_PARAMETERS;
                    break;
                }
                if (teec_param->memref.parent->size &&
                        (teec_param->memref.parent->flags & TEEC_MEM_INPUT)) {
                    LOG_D("set contents");
                    parent->set_contents(teec_param->memref.parent->buffer,
                                         teec_param->memref.parent->size);
                }
                break;
            }
            case TEEC_MEMREF_PARTIAL_OUTPUT:
            case TEEC_MEMREF_PARTIAL_INPUT:
            case TEEC_MEMREF_PARTIAL_INOUT: {
                LOG_D("  param #%zu, TEEC_MEMREF_PARTIAL_* %p/%zu/%x %zu/%zu", param_no,
                      teec_param->memref.parent->buffer, teec_param->memref.parent->size,
                      teec_param->memref.parent->flags, teec_param->memref.size,
                      teec_param->memref.offset);
                // Check data flow consistency
                if (((teec_param->memref.parent->flags & TEEC_MEM_INOUT) == TEEC_MEM_INPUT) &&
                        (param_type == TEEC_MEMREF_PARTIAL_OUTPUT)) {
                    LOG_E("PARTIAL data flow inconsistency (in)");
                    teec_result = TEEC_ERROR_BAD_PARAMETERS;
                    break;
                }

                if (((teec_param->memref.parent->flags & TEEC_MEM_INOUT) == TEEC_MEM_OUTPUT) &&
                        (param_type == TEEC_MEMREF_PARTIAL_INPUT)) {
                    LOG_E("PARTIAL data flow inconsistency (out)");
                    teec_result = TEEC_ERROR_BAD_PARAMETERS;
                    break;
                }

                if (teec_param->memref.offset + teec_param->memref.size >
                        teec_param->memref.parent->size) {
                    LOG_E("PARTIAL offset/size error: offset = %zu + size = %zu > parent size = %zu",
                          teec_param->memref.offset, teec_param->memref.size,
                          teec_param->memref.parent->size);
                    teec_result = TEEC_ERROR_BAD_PARAMETERS;
                    break;
                }
                GpParam* param = proxy_operation->add_params();
                GpRegisteredMemref* memref = param->mutable_memref();
                // Parent
                GpSharedMemory* parent = memref->mutable_parent();
                parent->set_buffer(reinterpret_cast<uintptr_t>
                                   (teec_param->memref.parent->buffer));
                parent->set_size(teec_param->memref.parent->size);
                parent->set_flags(teec_param->memref.parent->flags);
                // Reference
                memref->set_offset(teec_param->memref.offset);
                memref->set_size(teec_param->memref.size);
                std::lock_guard<std::mutex> lock(gp_shared_memories_mutex);
                auto it = std::find(gp_shared_memories.begin(), gp_shared_memories.end(),
                                    teec_param->memref.parent);
                if (it == gp_shared_memories.end()) {
                    LOG_E("shared memory not registered");
                    teec_result = TEEC_ERROR_BAD_PARAMETERS;
                    break;
                }
                if ((param_type != TEEC_MEMREF_PARTIAL_OUTPUT) && teec_param->memref.size) {
                    auto buffer = static_cast<const char*>(teec_param->memref.parent->buffer);
                    LOG_D("set contents");
                    parent->set_contents(&buffer[teec_param->memref.offset],
                            teec_param->memref.size);
                }
                break;
            }
            default:
                LOG_E("param #%zu, default", param_no);
                teec_result = TEEC_ERROR_BAD_PARAMETERS;
                break;
        }
    }
    EXIT(teec_result);
}

bool Proxy::Impl::parseOperation(
    const GpOperation& proxy_operation,
    TEEC_Operation* teec_operation) {
    ENTER();
    if (!proxy_operation.has_started() || !proxy_operation.has_param_types()) {
        LOG_E("Required parameter missing");
        EXIT(false);
    }
    teec_operation->started = proxy_operation.started();
    teec_operation->paramTypes = proxy_operation.param_types();
    // Parse parameters
    int params_index = 0;
    for (size_t param_no = 0; param_no < 4; param_no++) {
        TEEC_Parameter* teec_param = &teec_operation->params[param_no];
        uint32_t param_type = GET_PARAM_TYPE(teec_operation->paramTypes, param_no);

        switch (param_type) {
            case TEEC_NONE:
                LOG_D("  param #%zu, TEEC_NONE", param_no);
                break;
            case TEEC_VALUE_INPUT:
                LOG_D("  param #%zu, TEEC_VALUE_INPUT", param_no);
                break;
            case TEEC_VALUE_OUTPUT:
            case TEEC_VALUE_INOUT: {
                LOG_D("  param #%zu, TEEC_VALUE_OUT*", param_no);
                if (params_index > proxy_operation.params_size()) {
                    LOG_E("Too few parameter buffers");
                    EXIT(false);
                }
                const GpParam& param = proxy_operation.params(params_index++);
                const GpValue& value = param.value();
                teec_param->value.a = value.a();
                teec_param->value.b = value.b();
                break;
            }
            case TEEC_MEMREF_TEMP_INPUT:
            case TEEC_MEMREF_TEMP_OUTPUT:
            case TEEC_MEMREF_TEMP_INOUT: {
                LOG_D("  param #%zu, TEEC_MEMREF_TEMP_*", param_no);
                if (params_index > proxy_operation.params_size()) {
                    LOG_E("Too few parameter buffers");
                    EXIT(false);
                }
                const GpParam& param = proxy_operation.params(params_index++);
                const GpTempMemref& tmpref = param.tmpref();
                // Size is always returned:
                // * bytes returned by callee, or
                // * size needed to return the data
                teec_param->tmpref.size = tmpref.size();
                if (tmpref.has_contents()) {
                    LOG_D("get contents %zu", teec_param->tmpref.size);
                    ::memcpy(teec_param->tmpref.buffer, tmpref.contents().c_str(), tmpref.size());
                }
                break;
            }
            case TEEC_MEMREF_WHOLE: {
                LOG_D("  param #%zu, TEEC_MEMREF_WHOLE", param_no);
                if (params_index > proxy_operation.params_size()) {
                    LOG_E("Too few parameter buffers");
                    EXIT(false);
                }
                const GpParam& param = proxy_operation.params(params_index++);
                const GpRegisteredMemref& memref = param.memref();
                const GpSharedMemory& parent = memref.parent();
                // Size is always returned:
                // * bytes returned by callee, or
                // * size needed to return the data
                teec_param->memref.size = memref.size();
                if (parent.has_contents()) {
                    LOG_D("get contents %zu", parent.contents().size());
                    ::memcpy(teec_param->memref.parent->buffer, parent.contents().c_str(),
                             parent.contents().size());
                }
                break;
            }
            case TEEC_MEMREF_PARTIAL_INPUT:
            case TEEC_MEMREF_PARTIAL_OUTPUT:
            case TEEC_MEMREF_PARTIAL_INOUT: {
                LOG_D("  param #%zu, TEEC_MEMREF_PARTIAL_*", param_no);
                if (params_index > proxy_operation.params_size()) {
                    LOG_E("Too few parameter buffers");
                    EXIT(false);
                }
                const GpParam& param = proxy_operation.params(params_index++);
                const GpRegisteredMemref& memref = param.memref();
                const GpSharedMemory& parent = memref.parent();
                // Reference
                // Size is always returned:
                // * bytes returned by callee, or
                // * size needed to return the data
                teec_param->memref.size = memref.size();
                if (parent.has_contents()) {
                    auto parent_buffer = static_cast<char*>(teec_param->memref.parent->buffer);
                    LOG_D("get contents %zu", parent.contents().size());
                    ::memcpy(&parent_buffer[teec_param->memref.offset], parent.contents().c_str(),
                             parent.contents().size());
                }
                break;
            }
            default:
                LOG_E("param #%zu, default", param_no);
                EXIT(false);
        }
    }
    EXIT(true);
}

TEEC_Result Proxy::TEEC_InitializeContext(
    const char*             name,
    TEEC_Context*           /*context*/) {
    ENTER();
    std::lock_guard<std::mutex> lock(pimpl_->init_mutex);
    if (pimpl_->init_count > 0) {
        LOG_D("init_count: %d increment", pimpl_->init_count);
        pimpl_->init_count++;
        EXIT(TEEC_SUCCESS);
    }

    LOG_D("init_count: %d open", pimpl_->init_count);
    TEEC_Result res = TEEC_ERROR_GENERIC;
    pimpl_->client.registerPlatformContext(pimpl_->jvm, pimpl_->application_context);
    // Open proxy connection
    bool res_set_by_remote = false;
    if (pimpl_->client.open()) {
        GpInitializeContextRequest request;
        if (name) {
            request.set_name(name);
        }
        GpInitializeContextResponse response;
        GpCaller caller;
        // Note: call returns 0 on success
        if (!pimpl_->client.call(caller, GP_INITIALIZE_CONTEXT, request, &response)) {
            res = response.return_value();
            res_set_by_remote = true;
        }
    }

    if (!res_set_by_remote) {
        switch (errno) {
            case EACCES:
                res = TEEC_ERROR_ACCESS_DENIED;
                break;
            case EINVAL:
                res = TEEC_ERROR_BAD_PARAMETERS;
                break;
            case ENXIO:
            case ENOENT:
            case ECONNREFUSED:
                res = TEEC_ERROR_COMMUNICATION;
                break;
            default:
                res = TEEC_ERROR_GENERIC;
        }
    }

    if (res == TEEC_SUCCESS) {
        // Check NWd version
        mcVersionInfo_t version_info;
        if (mcGetMobiCoreVersion(MC_DEVICE_ID_DEFAULT, &version_info) == MC_DRV_OK) {
            if (version_info.versionCmp >= PROXY_COPY_MAX_VERSION) {
                pimpl_->quirk_session_auto_close = false;
            }
        }

        pimpl_->init_count = 1;
        LOG_I("Connected to proxy server");
        LOG_D("init_count: set to %d", pimpl_->init_count);
    }

    EXIT(res);
}

void Proxy::TEEC_FinalizeContext(
    TEEC_Context*           /*context*/) {
    ENTER();
    std::lock_guard<std::mutex> lock(pimpl_->init_mutex);
    if (pimpl_->init_count == 0) {
        LOG_E("init counter cannot go negative");
        EXIT_NORETURN();
    }

    LOG_D("init_count: %d decrement", pimpl_->init_count);
    if (--pimpl_->init_count == 0) {
        LOG_D("init_count: %d close", pimpl_->init_count);
        pimpl_->client.close();
    }

    EXIT_NORETURN();
}

TEEC_Result Proxy::TEEC_RegisterSharedMemory(
    TEEC_Context*           /*context*/,
    TEEC_SharedMemory*      sharedMem) {
    ENTER();
    TEEC_Result res = TEEC_SUCCESS;
    std::lock_guard<std::mutex> lock(pimpl_->gp_shared_memories_mutex);
    pimpl_->gp_shared_memories.push_back(*sharedMem);
    EXIT(res);
}

TEEC_Result Proxy::TEEC_AllocateSharedMemory(
    TEEC_Context*           context,
    TEEC_SharedMemory*      sharedMem) {
    if (!pimpl_->gp_allocated_shared_memories.allocate(sharedMem)) {
        LOG_E("mmap failed");
        return TEEC_ERROR_OUT_OF_MEMORY;
    }
    auto res = TEEC_RegisterSharedMemory(context, sharedMem);
    if (res != TEEC_SUCCESS) {
        pimpl_->gp_allocated_shared_memories.free(sharedMem);
    }
    return res;
}

void Proxy::TEEC_ReleaseSharedMemory(
    TEEC_SharedMemory*      sharedMem) {
    ENTER();
    {
        std::lock_guard<std::mutex> lock(pimpl_->gp_shared_memories_mutex);
        auto it = std::find(pimpl_->gp_shared_memories.begin(),
                            pimpl_->gp_shared_memories.end(), sharedMem);
        if (it != pimpl_->gp_shared_memories.end()) {
            pimpl_->gp_shared_memories.erase(it);
        }
    }
    pimpl_->gp_allocated_shared_memories.free(sharedMem);
    EXIT_NORETURN();
}

// Need to share current operation status (started field) for cancellation
class _TEECOperation {
    uint32_t* started_ = nullptr;
    enum State {
        STARTING,
        CONFIRMED,
        ENDED,
    };
    std::mutex mutex_;
    std::condition_variable cond_;
    void setState(State state) {
        std::lock_guard<std::mutex> lock(mutex_);
        LOG_D("shared operation %p changes state to %d", this, state);
        *started_ = (*started_ & ~0xf) | state;
        cond_.notify_all();
    }

    State getState() {
        return static_cast<State>(*started_ & 0xf);
    }

public:
    _TEECOperation(uint32_t* started) {
        started_ = started;
        LOG_D("shared operation %p starts, ID %u", this, *started_ >> 4);
    }

    ~_TEECOperation() {
        setState(State::ENDED);
    }

    uint32_t started() const {
        return *started_;
    }

    void confirm() {
        setState(State::CONFIRMED);
    }

    uint32_t wait() {
        std::unique_lock<std::mutex> lock(mutex_);
        while (getState() == State::STARTING) {
            cond_.wait(lock);
        }
        if (getState() == State::CONFIRMED) {
            LOG_D("shared operation %p will be cancelled, ID %u", this, *started_ >> 4);
            return *started_;
        }
        LOG_D("shared operation %p will not be cancelled", this);
        return 0;
    }
};

class _TEECOperations {
    std::vector<std::shared_ptr<_TEECOperation>> operations_;
    std::mutex mutex_;
public:
    void add(uint32_t* started) {
        std::lock_guard<std::mutex> lock(mutex_);
        operations_.emplace_back(std::make_shared<_TEECOperation>(started));
    }

    void remove(uint32_t started) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = std::find_if(operations_.begin(), operations_.end(),
                               [started](const std::shared_ptr<_TEECOperation>& operation) {
            return started == operation->started();
        });
        if (it != operations_.end()) {
            operations_.erase(it);
        }
    }

    std::shared_ptr<_TEECOperation> find(uint32_t started) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = std::find_if(operations_.begin(), operations_.end(),
                               [started](const std::shared_ptr<_TEECOperation>& operation) {
            return started == operation->started();
        });
        if (it == operations_.end()) {
            return nullptr;
        }
        return *it;
    }
};

static _TEECOperations shared_operations;

static void dummy_callback() {
    LOG_D("dummy callback called");
}

TEEC_Result Proxy::TEEC_OpenSession(
    TEEC_Context*           context,
    TEEC_Session*           session,
    const TEEC_UUID*        destination,
    uint32_t                connectionMethod,
    const void*             connectionData,
    TEEC_Operation*         operation,
    uint32_t*               returnOrigin) {
    ENTER();

    if (operation) {
        if (!pimpl_->operations.start(operation)) {
            if (returnOrigin) {
                *returnOrigin = TEEC_ORIGIN_COMMS;
            }

            return TEEC_ERROR_CANCEL;
        }

        shared_operations.add(&operation->started);
    }

    auto res = pimpl_->TEEC_OpenSession(
            context, session, destination, connectionMethod, connectionData,
            operation, returnOrigin);

    if (operation) {
        pimpl_->operations.end(operation);
    }

    EXIT(res);
}

TEEC_Result Proxy::Impl::TEEC_OpenSession(
    TEEC_Context*           /*context*/,
    TEEC_Session*           session,
    const TEEC_UUID*        destination,
    uint32_t                connectionMethod,
    const void*             connectionData,
    TEEC_Operation*         operation,
    uint32_t*               returnOrigin) {
    ENTER();
    TEEC_Result res = TEEC_SUCCESS;
    // Default returnOrigin is already set
    GpOpenSessionRequest request;
    request.set_uuid(destination, sizeof(*destination));
    request.set_login_type(static_cast<LoginType>(connectionMethod));
    if (connectionData) {
        request.set_login_data(connectionData, 16);
    }
    if (operation) {
        res = serializeOperation(operation, request.mutable_operation());
        if (res != TEEC_SUCCESS) {
            shared_operations.remove(operation->started);
            EXIT(res);
        }
    }

    GpOpenSessionResponse response;

    if (returnOrigin) {
        *returnOrigin = TEEC_ORIGIN_COMMS;
    }

    GpCaller caller(&response);
    std::shared_ptr<_TEECOperation> shared_operation;
    std::function<void()> confirm_cb;
    if (operation) {
        shared_operation = shared_operations.find(operation->started);
        confirm_cb = std::bind(&_TEECOperation::confirm, shared_operation.get());
    } else {
        confirm_cb = dummy_callback;
    }

    if (client.call(caller, GP_OPEN_SESSION, request, &response, confirm_cb)) {
        res = TEEC_ERROR_COMMUNICATION;
    }

    if (shared_operation) {
        shared_operations.remove(operation->started);
    }

    if (res != TEEC_SUCCESS) {
        EXIT(res);
    }

    if (operation && !parseOperation(response.operation(), operation)) {
        EXIT(TEEC_ERROR_COMMUNICATION);
    }

    if (response.return_value() == TEEC_SUCCESS) {
        session->imp.sessionId = response.session_id();
    }

    if (returnOrigin) {
        *returnOrigin = response.return_origin();
    }

    EXIT(response.return_value());
}

void Proxy::TEEC_CloseSession(
    TEEC_Session*           session) {
    ENTER();
    GpCloseSessionRequest request;
    request.set_session_id(session->imp.sessionId);
    EmptyMessage response;
    GpCaller caller(request.session_id(), true);
    pimpl_->client.call(caller, GP_CLOSE_SESSION, request, &response);
    EXIT_NORETURN();
}

TEEC_Result Proxy::TEEC_InvokeCommand(
    TEEC_Session*           session,
    uint32_t                commandID,
    TEEC_Operation*         operation,
    uint32_t*               returnOrigin) {
    ENTER();

    if (operation) {
        if (!pimpl_->operations.start(operation)) {
            if (returnOrigin) {
                *returnOrigin = TEEC_ORIGIN_COMMS;
            }

            return TEEC_ERROR_CANCEL;
        }

        shared_operations.add(&operation->started);
    }

    auto res = pimpl_->TEEC_InvokeCommand(
            session, commandID, operation, returnOrigin);

    if (operation) {
        pimpl_->operations.end(operation);
    }

    EXIT(res);
}

TEEC_Result Proxy::Impl::TEEC_InvokeCommand(
    TEEC_Session*           session,
    uint32_t                commandID,
    TEEC_Operation*         operation,
    uint32_t*               returnOrigin) {
    ENTER();
    TEEC_Result res = TEEC_SUCCESS;
    // Default returnOrigin is already set
    GpInvokeCommandRequest request;
    request.set_session_id(session->imp.sessionId);
    request.set_command_id(commandID);
    if (operation) {
        res = serializeOperation(operation, request.mutable_operation());
        if (res != TEEC_SUCCESS) {
            shared_operations.remove(operation->started);
            EXIT(res);
        }
    }

    GpInvokeCommandResponse response;
    GpCaller caller(request.session_id());
    LOG_D("session ID %x command ID %u", session->imp.sessionId, commandID);
    std::shared_ptr<_TEECOperation> shared_operation;
    std::function<void()> confirm_cb;
    if (operation) {
        shared_operation = shared_operations.find(operation->started);
        confirm_cb = std::bind(&_TEECOperation::confirm, shared_operation.get());
    } else {
        confirm_cb = dummy_callback;
    }

    if (client.call(caller, GP_INVOKE_COMMAND, request, &response, confirm_cb)) {
        switch (errno) {
            case ENXIO:
                res = TEEC_ERROR_BAD_STATE;
                break;
            default:
                res = TEEC_ERROR_COMMUNICATION;
        }
    }

    if (shared_operation) {
        shared_operations.remove(operation->started);
    }

    // Always parse operation, but do not override an error code
    if (operation && !parseOperation(response.operation(), operation) &&
            (res == TEEC_SUCCESS)) {
        res = TEEC_ERROR_COMMUNICATION;
    }

    if (res == TEEC_SUCCESS) {
        if (returnOrigin) {
            *returnOrigin = response.return_origin();
        }
        res = response.return_value();
        if ((res != TEEC_SUCCESS) && quirk_session_auto_close) {
            if ((res == TEEC_ERROR_COMMUNICATION) || (res == TEEC_ERROR_TARGET_DEAD) ||
                    (res == TEEC_ERROR_TARGET_KILLED)) {
                client.forceCloseConnection(caller);
                LOG_I("session %x automatically closed by client library (%x)",
                      session->imp.sessionId, res);
                ::memset(session, 0, sizeof(*session));
            }
        }
    }

    EXIT(res);
}

void Proxy::TEEC_RequestCancellation(
    TEEC_Operation*         operation) {
    ENTER();
    if (!pimpl_->operations.cancel(operation)) {
        EXIT_NORETURN();
    }

    auto shared_operation = shared_operations.find(operation->started);
    if (!shared_operation) {
        return;
    }

    auto started = shared_operation->wait();
    if (!started) {
        return;
    }

    GpRequestCancellationRequest request;
    request.set_started(started);
    EmptyMessage response;
    // Use a global connection as TEEC_InvokeCommand already uses the session's
    // or we are calling TEEC_OpenSession
    GpCaller caller;
    pimpl_->client.call(caller, GP_REQUEST_CANCELLATION, request, &response);
    EXIT_NORETURN();
}
