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

#include <vector>

#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <tee_client_api.h>     // TEEC_UUID
#include "mc_user.h"

#include "dynamic_log.h"
#include "driver.h"

// Global Platform macros
#define GET_PARAM_TYPE(t, i)    (((t) >> (4*i)) & 0xF)
#define PARAMS_NUMBER           4

struct Driver::Impl {
    int fd = -1;
    uint32_t min_accepted_version_;

    Impl(uint32_t v): min_accepted_version_(v) {}

    bool open() {
        int local_fd = ::open("/dev/" MC_USER_DEVNODE, O_RDWR | O_CLOEXEC);
        if (local_fd < 0) {
            LOG_E("%s in %s", strerror(errno), __FUNCTION__);
            return false;
        }

        struct mc_version_info version_info;
        if (::ioctl(local_fd, MC_IO_VERSION, &version_info) < 0) {
            LOG_E("%s in %s", strerror(errno), __FUNCTION__);
            (void)::close(local_fd);
            return false;
        }

        unsigned int major = version_info.version_nwd >> 16;
        unsigned int minor = version_info.version_nwd & 0xffff;
        bool version_mismatch = false;
        if (min_accepted_version_) {
            // driver-major should be equal to min_accepted_version_-major and
            // driver-minor should be greater than or equal to min_accepted_version_-minor
            unsigned int major_min_accepted_version = min_accepted_version_ >> 16;
            unsigned int minor_min_accepted_version = min_accepted_version_ & 0xffff;
            if (major != major_min_accepted_version) {
                version_mismatch = true;
                LOG_E("%s in %s (driver version mismatch detected: expected version major %d, got %d)",
                      strerror(errno), __FUNCTION__, major_min_accepted_version,
                      major);
            } else if (minor < minor_min_accepted_version) {
                version_mismatch = true;
                LOG_E("%s in %s (driver version mismatch detected: min expected version v%d.%d, got v%d.%d)",
                      strerror(errno), __FUNCTION__, major_min_accepted_version,
                      minor_min_accepted_version, major, minor);
            }
        } else {
            // The driver's version should match the MCDRVMODULEAPI's version
            if ((major != MCDRVMODULEAPI_VERSION_MAJOR) || (minor != MCDRVMODULEAPI_VERSION_MINOR)) {
                version_mismatch = true;
                LOG_E("%s in %s (driver version mismatch detected: expected v%d.%d, got v%d.%d)",
                      strerror(errno), __FUNCTION__, MCDRVMODULEAPI_VERSION_MAJOR,
                      MCDRVMODULEAPI_VERSION_MINOR, major, minor);
            }
        }
        if (version_mismatch) {
            (void)::close(local_fd);
            errno = EHOSTDOWN;
            return false;
        } else {
            LOG_I("driver version: v%d.%d", major, minor);
        }

        fd = local_fd;
        LOG_D("fd %d", fd);
        LOG_I("driver open");
        return true;
    }

    bool close() {
        int ret = ::close(fd);
        if (ret) {
            LOG_E("%s in %s", strerror(errno), __FUNCTION__);
        }
        LOG_D("fd %d", fd);
        fd = -1;
        LOG_I("driver closed");
        return ret == 0;
    }

    bool isOpen() const {
        return fd != -1;
    }

    bool hasOpenSessions() const {
        int ret = ::ioctl(fd, MC_IO_HAS_SESSIONS);
        if (ret) {
            LOG_E("%s in %s", strerror(errno), __FUNCTION__);
        }
        return ret != 0;
    }
};

Driver::Driver(uint32_t min_accepted_version):
    pimpl_(new Impl(min_accepted_version)) {
}

Driver::~Driver() {
    if (pimpl_->isOpen()) {
        pimpl_->close();
    }
}

// Proprietary
void Driver::TEEC_TT_RegisterPlatformContext(
    void*                   /*globalContext*/,
    void*                   /*localContext*/) {
    ENTER();
    EXIT_NORETURN();
}

mcResult_t Driver::TEEC_TT_TestEntry(
    void*                   /*buff*/,
    size_t                  /*len*/,
    uint32_t*               /*tag*/) {
    ENTER();
    EXIT(TEEC_ERROR_NOT_SUPPORTED);
}

// Global Platform
static void _libUuidToArray(
    uint8_t*         uuidArr,
    const TEEC_UUID* uuid) {
    uint8_t* pIdentifierCursor = (uint8_t*)uuid;
    /* offsets and syntax constants. See explanations above */
#ifdef S_BIG_ENDIAN
    uint32_t offsets = 0;
#else
    uint32_t offsets = 0xF1F1DF13;
#endif
    uint32_t i;

    for (i = 0; i < sizeof(TEEC_UUID); i++) {
        /* Two-digit hex number */
        uint8_t number;
        int32_t offset = ((int32_t)((offsets & 0xF) << 28)) >> 28;
        number = pIdentifierCursor[offset];
        offsets >>= 4;
        pIdentifierCursor++;

        uuidArr[i] = number;
    }
}

static TEEC_Result _TEEC_OperationToIoctl(
    struct gp_operation*        operation_int,
    const TEEC_Operation*       operation) {
    ENTER();
    operation_int->param_types = 0;
    if (!operation) {
        return TEEC_SUCCESS;
    }

    TEEC_Result teec_result = TEEC_SUCCESS;
    operation_int->started = operation->started;
    operation_int->param_types = operation->paramTypes;
    for (size_t param_no = 0; param_no < PARAMS_NUMBER; param_no++) {
        const TEEC_Parameter* param_in = &operation->params[param_no];
        union gp_param* param_out = &operation_int->params[param_no];
        uint32_t param_type = GET_PARAM_TYPE(operation->paramTypes, param_no);

        switch (param_type) {
            case TEEC_VALUE_OUTPUT:
                LOG_D("  param #%zu, TEEC_VALUE_OUTPUT", param_no);
                break;
            case TEEC_NONE:
                LOG_D("  param #%zu, TEEC_NONE", param_no);
                break;
            case TEEC_VALUE_INPUT:
            case TEEC_VALUE_INOUT: {
                LOG_D("  param #%zu, TEEC_VALUE_IN*", param_no);
                param_out->value.a = param_in->value.a;
                param_out->value.b = param_in->value.b;
                break;
            }
            case TEEC_MEMREF_TEMP_INPUT:
            case TEEC_MEMREF_TEMP_OUTPUT:
            case TEEC_MEMREF_TEMP_INOUT: {
                // A Temporary Memory Reference may be null, which can be used
                // to denote a special case for the parameter. Output Memory
                // References that are null are typically used to request the
                // required output size.
                LOG_D("  param #%zu, TEEC_MEMREF_TEMP_* %p/%zu",
                      param_no,
                      param_in->tmpref.buffer,
                      param_in->tmpref.size);
                param_out->tmpref.buffer = (uintptr_t)param_in->tmpref.buffer;
                param_out->tmpref.size = param_in->tmpref.size;
                break;
            }
            case TEEC_MEMREF_WHOLE: {
                LOG_D("  param #%zu, TEEC_MEMREF_WHOLE %p/%zu/%x",
                      param_no,
                      param_in->memref.parent->buffer,
                      param_in->memref.parent->size,
                      param_in->memref.parent->flags);
                param_out->memref.parent.buffer = (uintptr_t)param_in->memref.parent->buffer;
                param_out->memref.parent.size = param_in->memref.parent->size;
                param_out->memref.parent.flags = param_in->memref.parent->flags &
                                                 TEEC_MEM_FLAGS_MASK;
                break;
            }
            case TEEC_MEMREF_PARTIAL_INPUT:
            case TEEC_MEMREF_PARTIAL_OUTPUT:
            case TEEC_MEMREF_PARTIAL_INOUT: {
                LOG_D("  param #%zu, TEEC_MEMREF_PARTIAL_* %p/%zu/%x,%zu/%zu",
                      param_no,
                      param_in->memref.parent->buffer,
                      param_in->memref.parent->size,
                      param_in->memref.parent->flags,
                      param_in->memref.size,
                      param_in->memref.offset);
                //Check data flow consistency
                if ((((param_in->memref.parent->flags & TEEC_MEM_INOUT) == TEEC_MEM_INPUT) &&
                        (param_type == TEEC_MEMREF_PARTIAL_OUTPUT)) ||
                        (((param_in->memref.parent->flags & TEEC_MEM_INOUT) == TEEC_MEM_OUTPUT) &&
                         (param_type == TEEC_MEMREF_PARTIAL_INPUT))) {
                    LOG_E("PARTIAL data flow inconsistency");
                    teec_result = TEEC_ERROR_BAD_PARAMETERS;
                    break;
                }

                if (param_in->memref.offset + param_in->memref.size >
                        param_in->memref.parent->size) {
                    LOG_E("PARTIAL offset/size error: offset = %zu + size = %zu > parent size = %zu",
                          param_in->memref.offset, param_in->memref.size, param_in->memref.parent->size);
                    teec_result = TEEC_ERROR_BAD_PARAMETERS;
                    break;
                }
                // Parent
                param_out->memref.parent.buffer = (uintptr_t)param_in->memref.parent->buffer;
                param_out->memref.parent.size = param_in->memref.parent->size;
                param_out->memref.parent.flags = param_in->memref.parent->flags &
                                                 TEEC_MEM_FLAGS_MASK;
                // Reference
                param_out->memref.offset = param_in->memref.offset;
                param_out->memref.size = param_in->memref.size;
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

// Send values back to caller
static TEEC_Result _TEEC_IoctlToOperation(
    TEEC_Operation*             operation,
    const struct gp_operation*  operation_int) {
    ENTER();
    if (!operation) {
        return TEEC_SUCCESS;
    }
    TEEC_Result teec_result = TEEC_SUCCESS;
    operation->started = operation_int->started;
    operation->paramTypes = operation_int->param_types;
    for (size_t param_no = 0; param_no < PARAMS_NUMBER; param_no++) {
        const union gp_param* param_in = &operation_int->params[param_no];
        TEEC_Parameter* param_out = &operation->params[param_no];
        uint32_t param_type = GET_PARAM_TYPE(operation->paramTypes, param_no);

        switch (param_type) {
            case TEEC_VALUE_INPUT:
                LOG_D("  param #%zu, TEEC_VALUE_INPUT", param_no);
                break;
            case TEEC_NONE:
                LOG_D("  param #%zu, TEEC_NONE", param_no);
                break;
            case TEEC_VALUE_OUTPUT:
            case TEEC_VALUE_INOUT: {
                LOG_D("  param #%zu, TEEC_VALUE_OUT*", param_no);
                param_out->value.a = param_in->value.a;
                param_out->value.b = param_in->value.b;
                break;
            }
            case TEEC_MEMREF_TEMP_INPUT:
            case TEEC_MEMREF_TEMP_OUTPUT:
            case TEEC_MEMREF_TEMP_INOUT: {
                // A Temporary Memory Reference may be null, which can be used
                // to denote a special case for the parameter. Output Memory
                // References that are null are typically used to request the
                // required output size.
                LOG_D("  param #%zu, TEEC_MEMREF_TEMP_*", param_no);
                param_out->tmpref.size = static_cast<size_t>(param_in->tmpref.size);
                break;
            }
            case TEEC_MEMREF_WHOLE: {
                LOG_D("  param #%zu, TEEC_MEMREF_WHOLE", param_no);
                if (param_in->memref.parent.flags & TEEC_MEM_OUTPUT) {
                    param_out->memref.size = static_cast<size_t>(param_in->memref.size);
                }
                break;
            }
            case TEEC_MEMREF_PARTIAL_INPUT:
            case TEEC_MEMREF_PARTIAL_OUTPUT:
            case TEEC_MEMREF_PARTIAL_INOUT: {
                LOG_D("  param #%zu, TEEC_MEMREF_PARTIAL_*", param_no);
                param_out->memref.size = static_cast<size_t>(param_in->memref.size);
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

static TEEC_Result TEEC_ConvertErrno() {
    TEEC_Result teec_result = TEEC_ERROR_GENERIC;
    switch (errno) {
        case EACCES:
        case EPERM:
            teec_result = TEEC_ERROR_ACCESS_DENIED;
            break;
        case ENOMEM:
            teec_result = TEEC_ERROR_OUT_OF_MEMORY;
            break;
        case EINVAL:
            teec_result = TEEC_ERROR_BAD_PARAMETERS;
            break;
        case ENOENT:
            teec_result = TEEC_ERROR_COMMUNICATION;
            break;
    }
    return teec_result;
}

TEEC_Result Driver::TEEC_InitializeContext(
    const char*             /*name*/,
    TEEC_Context*           /*context*/) {
    ENTER();
    TEEC_Result teec_result = TEEC_SUCCESS;
    if (!pimpl_->open()) {
        teec_result = TEEC_ConvertErrno();
        LOG_E("open failed");
    } else {
        struct mc_ioctl_gp_initialize_context context;
        int ret = ::ioctl(pimpl_->fd, MC_IO_GP_INITIALIZE_CONTEXT, &context);
        if (ret && (ret != ECHILD)) {
            teec_result = TEEC_ConvertErrno();
            LOG_E("%s in %s", strerror(errno), __FUNCTION__);
        } else {
            teec_result = context.ret.value;
        }
    }
    EXIT(teec_result);
}

void Driver::TEEC_FinalizeContext(
    TEEC_Context*           /*context*/) {
    ENTER();
    pimpl_->close();
    EXIT_NORETURN();
}

TEEC_Result Driver::TEEC_RegisterSharedMemory(
    TEEC_Context*           /*context*/,
    TEEC_SharedMemory*      sharedMem) {
    ENTER();
    TEEC_Result teec_result = TEEC_SUCCESS;
    LOG_D("%p/%zu/%x", sharedMem->buffer, sharedMem->size, sharedMem->flags);
    struct mc_ioctl_gp_register_shared_mem shared_mem;
    shared_mem.memref.buffer = (uintptr_t)sharedMem->buffer;
    shared_mem.memref.size = sharedMem->size;
    shared_mem.memref.flags = sharedMem->flags;
    int ret = ::ioctl(pimpl_->fd, MC_IO_GP_REGISTER_SHARED_MEM, &shared_mem);
    if (ret && (ret != ECHILD)) {
        teec_result = TEEC_ConvertErrno();
        LOG_E("%s in %s", strerror(errno), __FUNCTION__);
    } else {
        teec_result = shared_mem.ret.value;
    }
    EXIT(teec_result);
}

TEEC_Result Driver::TEEC_AllocateSharedMemory(
    TEEC_Context*           /*context*/,
    TEEC_SharedMemory*      /*sharedMem*/) {
    return TEEC_ERROR_NOT_IMPLEMENTED;
}

void Driver::TEEC_ReleaseSharedMemory(
    TEEC_SharedMemory*      sharedMem) {
    ENTER();
    struct mc_ioctl_gp_release_shared_mem shared_mem;
    shared_mem.memref.buffer = (uintptr_t)sharedMem->buffer;
    shared_mem.memref.size = sharedMem->size;
    shared_mem.memref.flags = sharedMem->flags;
    int ret = ::ioctl(pimpl_->fd, MC_IO_GP_RELEASE_SHARED_MEM, &shared_mem);
    if (ret) {
        LOG_E("%s in %s", strerror(errno), __FUNCTION__);
    }
    EXIT_NORETURN();
}

TEEC_Result Driver::TEEC_OpenSession(
    TEEC_Context*           /*context*/,
    TEEC_Session*           session,
    const TEEC_UUID*        destination,
    uint32_t                connectionMethod,
    const void*             connectionData,
    TEEC_Operation*         operation,
    uint32_t*               returnOrigin) {
    ENTER();
    // Default returnOrigin is already set
    session->imp.sessionId = 0;

    struct mc_ioctl_gp_open_session session_int;
    memset(&session_int, 0, sizeof(session_int));

    TEEC_Result teec_result = _TEEC_OperationToIoctl(&session_int.operation,
                              operation);
    if (teec_result != TEEC_SUCCESS) {
        EXIT(teec_result);
    }
    _libUuidToArray(session_int.uuid.value, destination);
    session_int.identity.login_type = static_cast<mc_login_type>(connectionMethod);
    if (connectionData) {
        ::memcpy(&session_int.identity.login_data, connectionData,
                 sizeof(session_int.identity.login_data));
    }
    int ret = ::ioctl(pimpl_->fd, MC_IO_GP_OPEN_SESSION, &session_int);
    if (ret && (errno != ECHILD)) {
        teec_result = TEEC_ConvertErrno();
        LOG_E("%s in %s", strerror(errno), __FUNCTION__);
        EXIT(teec_result);
    }
    if (returnOrigin) {
        *returnOrigin = session_int.ret.origin;
    }
    teec_result = session_int.ret.value;
    _TEEC_IoctlToOperation(operation, &session_int.operation);
    if (teec_result == TEEC_SUCCESS) {
        session->imp.sessionId = session_int.session_id;
    }
    EXIT(teec_result);
}

void Driver::TEEC_CloseSession(
    TEEC_Session*           session) {
    ENTER();
    if (!session->imp.sessionId) {
        LOG_W("session is not active");
        return;
    }
    struct mc_ioctl_gp_close_session session_int;
    session_int.session_id = session->imp.sessionId;
    int ret = ::ioctl(pimpl_->fd, MC_IO_GP_CLOSE_SESSION, &session_int);
    if (ret) {
        LOG_E("%s in %s", strerror(errno), __FUNCTION__);
    }
    session->imp.sessionId = 0;
    EXIT_NORETURN();
}

TEEC_Result Driver::TEEC_InvokeCommand(
    TEEC_Session*           session,
    uint32_t                commandID,
    TEEC_Operation*         operation,
    uint32_t*               returnOrigin) {
    ENTER();
    // Default returnOrigin is already set
    TEEC_Result teec_result;
    if (!session->imp.sessionId) {
        teec_result = TEEC_ERROR_BAD_STATE;
        LOG_E("session is inactive");
        EXIT(teec_result);
    }

    struct mc_ioctl_gp_invoke_command command;
    command.session_id = session->imp.sessionId;
    command.command_id = commandID;
    teec_result = _TEEC_OperationToIoctl(&command.operation, operation);
    if (teec_result != TEEC_SUCCESS) {
        EXIT(teec_result);
    }
    int ret = ::ioctl(pimpl_->fd, MC_IO_GP_INVOKE_COMMAND, &command);
    if (ret && (errno != ECHILD)) {
        teec_result = TEEC_ConvertErrno();
        LOG_E("%s in %s", strerror(errno), __FUNCTION__);
        EXIT(teec_result);
    }
    teec_result = command.ret.value;
    _TEEC_IoctlToOperation(operation, &command.operation);
    if (returnOrigin) {
        *returnOrigin = command.ret.origin;
    }
    EXIT(teec_result);
}

void Driver::TEEC_RequestCancellation(
    TEEC_Operation*         operation) {
    ENTER();
    struct mc_ioctl_gp_request_cancellation cancellation;
    cancellation.operation.started = operation->started;
    int ret = ::ioctl(pimpl_->fd, MC_IO_GP_REQUEST_CANCELLATION, &cancellation);
    if (ret) {
        LOG_E("%s in %s", strerror(errno), __FUNCTION__);
    }
    EXIT_NORETURN();
}

// MobiCore
static mcResult_t ret_errno_to_mc_result(int32_t ret) {
    switch (ret) {
        case 0:
            return MC_DRV_OK;
        case -1:
            switch (errno) {
                case ENOMSG:
                    return MC_DRV_NO_NOTIFICATION;
                case EBADMSG:
                    return MC_DRV_ERR_NOTIFICATION;
                case ENOSPC:
                    return MC_DRV_ERR_OUT_OF_RESOURCES;
                case EHOSTDOWN:
                    return MC_DRV_ERR_INIT;
                case ENODEV:
                    return MC_DRV_ERR_UNKNOWN_DEVICE;
                case ENXIO:
                    return MC_DRV_ERR_UNKNOWN_SESSION;
                case EPERM:
                    return MC_DRV_ERR_INVALID_OPERATION;
                case EBADE:
                    return MC_DRV_ERR_INVALID_RESPONSE;
                case ETIME:
                    return MC_DRV_ERR_TIMEOUT;
                case ENOMEM:
                    return MC_DRV_ERR_NO_FREE_MEMORY;
                case EUCLEAN:
                    return MC_DRV_ERR_FREE_MEMORY_FAILED;
                case ENOTEMPTY:
                    return MC_DRV_ERR_SESSION_PENDING;
                case EHOSTUNREACH:
                    return MC_DRV_ERR_DAEMON_UNREACHABLE;
                case ENOENT:
                    return MC_DRV_ERR_INVALID_DEVICE_FILE;
                case EINVAL:
                    return MC_DRV_ERR_INVALID_PARAMETER;
                case EPROTO:
                    return MC_DRV_ERR_KERNEL_MODULE;
                case EFAULT:
                    return MC_DRV_ERR_BULK_MAPPING;
                case EADDRINUSE:
                    return MC_DRV_ERR_BULK_MAPPING;
                case EADDRNOTAVAIL:
                    return MC_DRV_ERR_BULK_UNMAPPING;
                case ECOMM:
                    return MC_DRV_INFO_NOTIFICATION;
                case EUNATCH:
                    return MC_DRV_ERR_NQ_FAILED;
                case EBADF:
                    return MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN;
                case EINTR:
                    return MC_DRV_ERR_INTERRUPTED_BY_SIGNAL;
                case EKEYREJECTED:
                    return MC_DRV_ERR_WRONG_PUBLIC_KEY;
                case ECONNREFUSED:
                    return MC_DRV_ERR_SERVICE_BLOCKED;
                case ECONNABORTED:
                    return MC_DRV_ERR_SERVICE_LOCKED;
                case ECONNRESET:
                    return MC_DRV_ERR_SERVICE_KILLED;
                case EBUSY:
                    return MC_DRV_ERR_NO_FREE_INSTANCES;
                default:
                    LOG_ERRNO("syscall");
                    return MC_DRV_ERR_UNKNOWN;
            }
        default:
            LOG_E("Unknown code %d", ret);
            return MC_DRV_ERR_UNKNOWN;
    }
}

mcResult_t Driver::mcOpenDevice(
    uint32_t                /*deviceId*/) {
    ENTER();
    if (pimpl_->open()) {
        EXIT(MC_DRV_OK);
    }
    switch (errno) {
        case EINVAL:
            EXIT(MC_DRV_ERR_INVALID_PARAMETER);
        case ENOENT:
            EXIT(MC_DRV_ERR_INVALID_DEVICE_FILE);
        case EPERM:
            EXIT(MC_DRV_ERR_INVALID_OPERATION);
    }
    EXIT(MC_DRV_ERR_UNKNOWN);
}

mcResult_t Driver::mcCloseDevice(
    uint32_t                /*deviceId*/) {
    ENTER();
    if (pimpl_->hasOpenSessions()) {
        EXIT(MC_DRV_ERR_SESSION_PENDING);
    }
    if (pimpl_->close()) {
        EXIT(MC_DRV_OK);
    }
    EXIT(MC_DRV_ERR_UNKNOWN);
}

mcResult_t Driver::mcOpenSession(
    mcSessionHandle_t*      session,
    const mcUuid_t*         uuid,
    uint8_t*                tci,
    uint32_t                tciLen) {
    ENTER();
    struct mc_ioctl_open_session sess;
    sess.sid = 0;
    ::memcpy(&sess.uuid, uuid, sizeof(sess.uuid));
    sess.tci = reinterpret_cast<uintptr_t>(tci);
    sess.tcilen = tciLen;
    sess.is_gp_uuid = false;
    sess.identity.login_type = LOGIN_PUBLIC;
    int ret = ::ioctl(pimpl_->fd, MC_IO_OPEN_SESSION, &sess);
    if (!ret) {
        session->sessionId = sess.sid;
        LOG_D("session %x open", session->sessionId);
    }
    mcResult_t mc_result = ret_errno_to_mc_result(ret);
    if (mc_result == MC_DRV_ERR_BULK_MAPPING) {
        // TCI issue
        mc_result = MC_DRV_ERR_INVALID_PARAMETER;
    }
    EXIT(mc_result);
}

mcResult_t Driver::mcOpenTrustlet(
    mcSessionHandle_t*      session,
    mcSpid_t                spid,
    uint8_t*                trustedapp,
    uint32_t                tLen,
    uint8_t*                tci,
    uint32_t                tciLen) {
    ENTER();
    struct mc_ioctl_open_trustlet trus;
    trus.sid = 0;
    trus.spid = spid;
    trus.buffer = reinterpret_cast<uintptr_t>(trustedapp);
    trus.tlen = tLen;
    trus.tci = reinterpret_cast<uintptr_t>(tci);
    trus.tcilen = tciLen;
    int ret = ::ioctl(pimpl_->fd, MC_IO_OPEN_TRUSTLET, &trus);
    if (!ret) {
        session->sessionId = trus.sid;
        LOG_D("session %x open", session->sessionId);
    }
    EXIT(ret_errno_to_mc_result(ret));
}

mcResult_t Driver::mcCloseSession(
    mcSessionHandle_t*      session) {
    ENTER();
    LOG_D("session %x close", session->sessionId);
    int ret = ::ioctl(pimpl_->fd, MC_IO_CLOSE_SESSION, session->sessionId);
    if (!ret) {
        LOG_D("session %x closed", session->sessionId);
    }
    EXIT(ret_errno_to_mc_result(ret));
}

mcResult_t Driver::mcNotify(
    mcSessionHandle_t*      session) {
    ENTER();
    LOG_D("session %x notify", session->sessionId);
    int ret = ::ioctl(pimpl_->fd, MC_IO_NOTIFY, session->sessionId);
    if (!ret) {
        LOG_D("session %x notified", session->sessionId);
    }
    EXIT(ret_errno_to_mc_result(ret));
}

mcResult_t Driver::mcWaitNotification(
    mcSessionHandle_t*      session,
    int32_t                 timeout) {
    ENTER();
    LOG_D("session %x wait for notification", session->sessionId);
    struct mc_ioctl_wait wait;
    wait.sid = session->sessionId;
    wait.timeout = timeout;
    int ret = ::ioctl(pimpl_->fd, MC_IO_WAIT, &wait);
    if (!ret) {
        LOG_D("session %x notification received", session->sessionId);
    }
    EXIT(ret_errno_to_mc_result(ret));
}

mcResult_t Driver::mcMallocWsm(
    uint32_t                /*deviceId*/,
    uint32_t                len,
    uint8_t**               wsm) {
    ENTER();
    int ret = 0;
    *wsm = static_cast<uint8_t*>(mmap(0, len, PROT_READ | PROT_WRITE,
                                      MAP_SHARED, pimpl_->fd, 0));
    if (*wsm == MAP_FAILED) {
        ret = -1;
    } else {
        LOG_D("wsm allocated");
    }
    EXIT(ret_errno_to_mc_result(ret));
}

mcResult_t Driver::mcFreeWsm(
    uint32_t                /*deviceId*/,
    uint8_t*                wsm,
    uint32_t                len) {
    ENTER();
    int ret = munmap(wsm, len);
    if (!ret) {
        LOG_D("wsm freed");
    }
    EXIT(ret_errno_to_mc_result(ret));
}

mcResult_t Driver::mcMap(
    mcSessionHandle_t*      session,
    void*                   buf,
    uint32_t                len,
    mcBulkMap_t*            mapInfo) {
    ENTER();
    LOG_D("session %x map va=%p len=%u", session->sessionId, buf, len);
    struct mc_ioctl_map map;
    map.sid = session->sessionId;
    map.buf.va = reinterpret_cast<uintptr_t>(buf);
    map.buf.len = len;
    map.buf.flags = MC_IO_MAP_INPUT_OUTPUT;
    int ret = ::ioctl(pimpl_->fd, MC_IO_MAP, &map);
    if (!ret) {
        LOG_D("session %x mapped va=%llx len=%u sva=%llx", map.sid,
              map.buf.va, map.buf.len, map.buf.sva);
#if ( __WORDSIZE != 64 )
        mapInfo->sVirtualAddr = reinterpret_cast<void*>(static_cast<uintptr_t>
                                (map.buf.sva));
#else
        mapInfo->sVirtualAddr = static_cast<uint32_t>(map.buf.sva);
#endif
        mapInfo->sVirtualLen = map.buf.len;
    }
    EXIT(ret_errno_to_mc_result(ret));
}

mcResult_t Driver::mcUnmap(
    mcSessionHandle_t*      session,
    void*                   buf,
    mcBulkMap_t*            mapInfo) {
    ENTER();
    struct mc_ioctl_map map;
    map.sid = session->sessionId;
    map.buf.va = reinterpret_cast<uintptr_t>(buf);
#if ( __WORDSIZE != 64 )
    map.buf.sva = reinterpret_cast<uintptr_t>(mapInfo->sVirtualAddr);
#else
    map.buf.sva = mapInfo->sVirtualAddr;
#endif
    map.buf.len = mapInfo->sVirtualLen;
    LOG_D("session %x unmap va=%llx len=%u sva=%llx", map.sid,
          map.buf.va, map.buf.len, map.buf.sva);
    int ret = ::ioctl(pimpl_->fd, MC_IO_UNMAP, &map);
    if (!ret) {
        LOG_D("session %x unmap va=%p len=%u sva=%llx", session->sessionId, buf,
              mapInfo->sVirtualLen, map.buf.sva);
    }
    EXIT(ret_errno_to_mc_result(ret));
}

mcResult_t Driver::mcGetSessionErrorCode(
    mcSessionHandle_t*      session,
    int32_t*                lastErr) {
    ENTER();
    LOG_D("session %x get error code", session->sessionId);
    struct mc_ioctl_geterr err;
    err.sid = session->sessionId;
    int ret = ::ioctl(pimpl_->fd, MC_IO_ERR, &err);
    if (!ret) {
        *lastErr = err.value;
        LOG_D("session %x error code %x", session->sessionId, *lastErr);
    }
    EXIT(ret_errno_to_mc_result(ret));
}

mcResult_t Driver::mcGetMobiCoreVersion(
    uint32_t                /*deviceId*/,
    mcVersionInfo_t*        versionInfo) {
    ENTER();
    LOG_D("get SWd versions");
    struct mc_version_info version_info;
    int ret = ::ioctl(pimpl_->fd, MC_IO_VERSION, &version_info);
    if (!ret) {
        ::memcpy(versionInfo, &version_info, sizeof(*versionInfo));
        LOG_D("got SWd versions");
    }
    EXIT(ret_errno_to_mc_result(ret));
}
