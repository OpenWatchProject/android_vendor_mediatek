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

#include <algorithm>
#include <memory>
#include <vector>

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>

#include "dynamic_log.h"
#include "MobiCoreDriverApi.h"
#include "proxy_internal.h"
#include "proxy_session.h"
#include "proxy_client.h"

using namespace trustonic;

//------------------------------------------------------------------------------

class SessionsManager {
    std::mutex sessions_mutex_;
    std::vector<std::shared_ptr<Session>> sessions_;
public:
    std::shared_ptr<Session> find(uint32_t id) {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        auto it = std::find_if(sessions_.begin(), sessions_.end(),
        [id](std::shared_ptr<Session>& session) {
            return session->id() == id;
        }
                              );
        if (it == sessions_.end()) {
            return nullptr;
        }
        return *it;
    }
    void add(const std::shared_ptr<Session>& session) {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        sessions_.push_back(session);
    }
    void remove(uint32_t id) {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        auto it = std::find_if(sessions_.begin(), sessions_.end(),
        [id](std::shared_ptr<Session>& session) {
            return session->id() == id;
        }
                              );
        if (it != sessions_.end()) {
            sessions_.erase(it);
        }
    }
    void clear() {
        if (!sessions_.empty()) {
            LOG_W("MC session list cleared");
            std::lock_guard<std::mutex> lock(sessions_mutex_);
            sessions_.clear();
        }
    }
};

class McCaller: public ProxyClient::ICaller {
    OpenSessionResponse* response_ = nullptr;
    uint32_t session_id_ = 0;
    bool is_close_session_ = false;
    bool is_device_operation_ = false;
    bool is_signalled_ = false;
public:
    McCaller(): is_device_operation_(true) {}
    McCaller(uint32_t session_id, bool is_close_session = false):
        session_id_(session_id), is_close_session_(is_close_session) {}
    McCaller(OpenSessionResponse* response): response_(response) {}
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
        return response_->id();
    }
    void setSignalled() override {
        is_signalled_ = true;
    }
    bool isSignalled() const {
        return is_signalled_;
    }
};

static SessionsManager sessions;

// Only 1 device is supported
static inline bool isValidDevice(uint32_t deviceId) {
    return (MC_DEVICE_ID_DEFAULT == deviceId);
}

// Convert driver errors into tbase set
static mcResult_t convert_syscall_error(int32_t ret) {
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

//------------------------------------------------------------------------------
//      CLIENT API
//------------------------------------------------------------------------------

// MobiCore
mcResult_t Proxy::mcOpenDevice(
    uint32_t                /*deviceId*/) {
    ENTER();
    mcResult_t mcResult = MC_DRV_ERR_DAEMON_UNREACHABLE;
    TEEC_Context context;
    auto res = TEEC_InitializeContext(nullptr, &context);
    switch (res) {
        case TEEC_SUCCESS:
            mcResult = MC_DRV_OK;
            break;
        case TEEC_ERROR_ACCESS_DENIED:
            mcResult = MC_DRV_ERR_INVALID_OPERATION;
            break;
    }

    EXIT(mcResult);
}

mcResult_t Proxy::mcCloseDevice(
    uint32_t                /*deviceId*/) {
    ENTER();
    // Check that all sessions are closed for client
    mcResult_t mcResult = convert_syscall_error(pimpl_->client.hasConnections());
    if (mcResult == MC_DRV_OK) {
        TEEC_FinalizeContext(nullptr);
    }

    EXIT(mcResult);
}

mcResult_t Proxy::mcOpenSession(
    mcSessionHandle_t*      session,
    const mcUuid_t*         uuid,
    uint8_t*                tci,
    uint32_t                tciLen) {
    ENTER();
    mcResult_t mcResult = MC_DRV_ERR_UNKNOWN;
    int ret;

    do {
        // Call
        OpenSessionRequest request;
        request.set_uuid(reinterpret_cast<const void*>(uuid), sizeof(*uuid));
        request.set_is_gp_uuid(false);
        if (tci) {
            request.set_tci(tci, tciLen);
        }
        request.set_login_type(TEE_PROXY_LOGIN_PUBLIC);
        char login_data[16];
        ::memset(login_data, 0, sizeof(login_data));
        request.set_login_data(login_data, sizeof(login_data));
        OpenSessionResponse response;
        McCaller caller(&response);
        ret = pimpl_->client.call(caller, OPEN_SESSION, request, &response);
        mcResult = convert_syscall_error(ret);
        if (MC_DRV_OK != mcResult) {
            break;
        }

        // Fill in return parameters
        session->sessionId = response.id();

        // Create session management object
        std::shared_ptr<Session::Buffer> buffer;
        if (tci) {
            buffer = std::make_shared<Session::Buffer>(tci, tciLen);
        }
        sessions.add(std::make_shared<Session>(session->sessionId, buffer));
        pimpl_->client.registerCloseCallback(std::bind(&SessionsManager::clear, &sessions));
    } while (false);

    // Legacy error matching
    if (MC_DRV_OK != mcResult) {
        if (MC_DRV_ERR_UNKNOWN == mcResult) {
            mcResult = MC_DRV_ERR_INVALID_PARAMETER;
        }
    }

    EXIT(mcResult);
}

mcResult_t Proxy::mcOpenTrustlet(
    mcSessionHandle_t*      session,
    mcSpid_t                spid,
    uint8_t*                trustedapp,
    uint32_t                tLen,
    uint8_t*                tci,
    uint32_t                tciLen) {
    ENTER();
    mcResult_t mcResult = MC_DRV_ERR_UNKNOWN;
    int ret;

    do {
        // Call
        OpenTrustletRequest request;
        request.set_spid(spid);
        request.set_trustapp(trustedapp, tLen);
        if (tci) {
            request.set_tci(tci, tciLen);
        }
        OpenSessionResponse response;
        McCaller caller(&response);
        ret = pimpl_->client.call(caller, OPEN_TRUSTLET, request, &response);
        mcResult = convert_syscall_error(ret);
        if (MC_DRV_OK != mcResult) {
            break;
        }

        // Fill in return parameters
        session->sessionId = response.id();

        // Create session management object
        std::shared_ptr<Session::Buffer> buffer;
        if (tci) {
            buffer = std::make_shared<Session::Buffer>(tci, tciLen);
        }
        sessions.add(std::make_shared<Session>(session->sessionId, buffer));
        pimpl_->client.registerCloseCallback(std::bind(&SessionsManager::clear, &sessions));
    } while (false);

    // Legacy error matching
    if (MC_DRV_OK != mcResult) {
        if (MC_DRV_ERR_UNKNOWN == mcResult) {
            mcResult = MC_DRV_ERR_INVALID_PARAMETER;
        }
    }

    EXIT(mcResult);
}

mcResult_t Proxy::mcCloseSession(
    mcSessionHandle_t*      session) {
    ENTER();
    mcResult_t mcResult = MC_DRV_ERR_UNKNOWN;
    int ret;

    LOG_D("session %x close", session->sessionId);
    do {
        // Call
        CloseSessionRequest request;
        request.set_id(session->sessionId);
        EmptyMessage response;
        McCaller caller(session->sessionId, true);
        ret = pimpl_->client.call(caller, CLOSE_SESSION, request, &response);
        mcResult = convert_syscall_error(ret);
    } while (false);

    if (MC_DRV_OK == mcResult) {
        LOG_D("session %x closed", session->sessionId);
        sessions.remove(session->sessionId);
    }

    EXIT(mcResult);
}

mcResult_t Proxy::mcNotify(
    mcSessionHandle_t*      session) {
    ENTER();
    mcResult_t mcResult = MC_DRV_ERR_UNKNOWN;
    int ret;

    do {
        // Get session
        auto s = sessions.find(session->sessionId);
        if (!s) {
            mcResult = MC_DRV_ERR_UNKNOWN_SESSION;
            break;
        }
        LOG_D("session %x notify", session->sessionId);

        // Call
        NotifyRequest request;
        request.set_sid(session->sessionId);
        if (s->hasTci()) {
            request.set_tci(s->tci(), s->tciLen());
        }
        auto buffers = s->buffers();
        for (auto& buf: buffers) {
            NotifyRequest_Buffers* buffer = request.add_buffers();
            buffer->set_sva(buf->sva());
            buffer->set_data(buf->address(), buf->length());
        }
        EmptyMessage response;
        McCaller caller(session->sessionId);
        ret = pimpl_->client.call(caller, NOTIFY, request, &response);
        mcResult = convert_syscall_error(ret);
    } while (false);

    EXIT(mcResult);
}

mcResult_t Proxy::mcWaitNotification(
    mcSessionHandle_t*      session,
    int32_t                 timeout) {
    ENTER();
    mcResult_t mcResult = MC_DRV_ERR_UNKNOWN;
    int ret;

    do {
        // Get session
        auto s = sessions.find(session->sessionId);
        if (!s) {
            mcResult = MC_DRV_ERR_UNKNOWN_SESSION;
            break;
        }
        LOG_D("session %x wait for notification", session->sessionId);

        // Call
        WaitNotificationRequest request;
        request.set_sid(session->sessionId);
        WaitNotificationResponse response;
        int32_t timeout_left = timeout;
        while (true) {
            // Do not block for a long time, retry every second
            if (timeout < 0) {
                // Infinite
                request.set_timeout(timeout_max);
                request.set_partial(true);
            } else if (timeout_left > timeout_max) {
                // Big
                request.set_timeout(timeout_max);
                request.set_partial(true);
                timeout_left -= timeout_max;
            } else {
                // Small enough
                request.set_timeout(timeout_left);
                request.set_partial(false);
                timeout_left = 0;
            }
            LOG_D("timeout: asked=%d left=%d set=%d",
                  timeout, timeout_left, request.timeout());

            // Exchange, stop on signal reveived
            McCaller caller(session->sessionId);
            ret = pimpl_->client.call(caller, WAIT, request, &response);
            if (ret) {
                if (caller.isSignalled()) {
                    mcResult = MC_DRV_ERR_INTERRUPTED_BY_SIGNAL;
                } else {
                    mcResult = convert_syscall_error(ret);
                }
            } else {
                mcResult = MC_DRV_OK;
            }

            bool retry;
            switch (mcResult) {
                case MC_DRV_ERR_TIMEOUT:
                    if (request.partial()) {
                        // Timed out but full requested timeout not expired
                        retry = true;
                    } else {
                        // Timeout expired
                        retry = false;
                    }
                    break;
                case MC_DRV_ERR_INTERRUPTED_BY_SIGNAL:
                    if (timeout == MC_INFINITE_TIMEOUT) {
                        // Interrupted, but timeout is 'hard' infinite
                        retry = true;
                    } else {
                        // Signal received
                        retry = false;
                    }
                    break;
                default:
                    // Other error: give up
                    retry = false;
                    break;
            }

            if (!retry) {
                break;
            }
            LOG_D("retry");
        }

        // Response
        if (response.has_tci() && s->updateTci(response.tci())) {
            LOG_E("Could not update TCI");
            mcResult = MC_DRV_ERR_UNKNOWN;
            break;
        }
        for (int i = 0; i < response.buffers_size(); i++) {
            const WaitNotificationResponse_Buffers& buffer = response.buffers(i);
            if (!buffer.has_sva() || !buffer.has_data()) {
                LOG_E("Required parameter missing");
                mcResult = MC_DRV_ERR_UNKNOWN;
                break;
            }
            auto buf = s->findBuffer(buffer.sva());
            if (!buf) {
                LOG_E("Buffer not found for SVA %llx", static_cast<unsigned long long>(buffer.sva()));
                mcResult = MC_DRV_ERR_UNKNOWN;
                break;
            }
            if (buffer.data().length() != buf->length()) {
                LOG_E("Buffer sizes differ for SVA %llx: %zu != %u",
                      static_cast<unsigned long long>(buffer.sva()), buffer.data().length(), buf->length());
                mcResult = MC_DRV_ERR_UNKNOWN;
                break;
            }
            ::memcpy(buf->address(), buffer.data().c_str(), buf->length());
        }
    } while (false);

    EXIT(mcResult);
}

mcResult_t Proxy::mcMallocWsm(
    uint32_t                /*deviceId*/,
    uint32_t                len,
    uint8_t**               wsm) {
    ENTER();
    mcResult_t mcResult = MC_DRV_ERR_UNKNOWN;

    do {
        if (!pimpl_->client.isOpen()) {
            LOG_E("Client not open");
            mcResult = MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN;
            break;
        }

        // Cannot share kernel buffers through the proxy
        *wsm = (uint8_t*)::mmap(0, len, PROT_READ | PROT_WRITE,
                                MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (*wsm == MAP_FAILED) {
            mcResult = MC_DRV_ERR_NO_FREE_MEMORY;
            break;
        }
        LOG_D("mmap'd wsm %p len %u", *wsm, len);
        mcResult = MC_DRV_OK;
    } while (false);

    EXIT(mcResult);
}

mcResult_t Proxy::mcFreeWsm(
    uint32_t                deviceId,
    uint8_t*                wsm,
    uint32_t                len) {
    ENTER();
    mcResult_t mcResult = MC_DRV_ERR_UNKNOWN;

    do {
        // Check parameters
        if (!isValidDevice(deviceId)) {
            mcResult = MC_DRV_ERR_UNKNOWN_DEVICE;
            break;
        }
        if (!pimpl_->client.isOpen()) {
            LOG_E("Client not open");
            mcResult = MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN;
            break;
        }
        LOG_D("munmap wsm %p len %u", wsm, len);
        if (::munmap(wsm, len)) {
            mcResult = convert_syscall_error(errno);
        } else {
            mcResult = MC_DRV_OK;
        }
    } while (false);

    EXIT(mcResult);
}

mcResult_t Proxy::mcMap(
    mcSessionHandle_t*      session,
    void*                   buf,
    uint32_t                len,
    mcBulkMap_t*            mapInfo) {
    ENTER();
    mcResult_t mcResult = MC_DRV_ERR_UNKNOWN;
    int ret;

    do {
        // Get session
        auto s = sessions.find(session->sessionId);
        if (!s) {
            mcResult = MC_DRV_ERR_UNKNOWN_SESSION;
            break;
        }

        // Call
        MapRequest request;
        request.set_sid(session->sessionId);
        auto buffer = request.add_buffers();
        buffer->set_va(reinterpret_cast<uintptr_t>(buf));
        buffer->set_len(len);
        // Useless but required
        buffer->set_flags(0);
        LOG_D("map va=%p len=%u", buf, len);
        MapResponse response;
        McCaller caller(session->sessionId);
        ret = pimpl_->client.call(caller, MAP, request, &response);
        mcResult = convert_syscall_error(ret);
        if (MC_DRV_OK != mcResult) {
            break;
        }

        // Fill in return parameters
        const MapResponse_Buffers& resp_buf = response.buffers(0);
        if (!resp_buf.has_sva()) {
            LOG_E("Required parameter missing");
            mcResult = MC_DRV_ERR_UNKNOWN;
            break;
        }
        auto sva = static_cast<uint32_t>(resp_buf.sva());
#if ( __WORDSIZE != 64 )
        mapInfo->sVirtualAddr = reinterpret_cast<void*>(sva);
#else
        mapInfo->sVirtualAddr = static_cast<uint32_t>(sva);
#endif
        mapInfo->sVirtualLen = len;
        LOG_D("map'd va=%p len=%u sva=%x", buf, mapInfo->sVirtualLen, sva);
        // Add buffer to session
        s->addBuffer(buffer->va(), buffer->len(), resp_buf.sva());
        LOG_D("session %x buffer %jx mapped", session->sessionId, resp_buf.sva());
    } while (false);

    // Legacy error matching
    if (MC_DRV_OK != mcResult) {
        if (MC_DRV_ERR_UNKNOWN == mcResult) {
            mcResult = MC_DRV_ERR_BULK_MAPPING;
        }
    }

    EXIT(mcResult);
}

mcResult_t Proxy::mcUnmap(
    mcSessionHandle_t*      session,
    void*                   buf,
    mcBulkMap_t*            mapInfo) {
    (void)buf;
    ENTER();
    mcResult_t mcResult = MC_DRV_ERR_UNKNOWN;
    int ret;

    do {
        // Get session
        auto s = sessions.find(session->sessionId);
        if (!s) {
            mcResult = MC_DRV_ERR_UNKNOWN_SESSION;
            break;
        }

        // Call
        UnmapRequest request;
        request.set_sid(session->sessionId);
        UnmapRequest_Buffers* buffer = request.add_buffers();
#if ( __WORDSIZE != 64 )
        auto sva = reinterpret_cast<uint32_t>(mapInfo->sVirtualAddr);
#else
        auto sva = mapInfo->sVirtualAddr;
#endif
        buffer->set_sva(sva);

        LOG_D("unmap va=%p len=%u sva=%x", buf, mapInfo->sVirtualLen, sva);
        EmptyMessage response;
        McCaller caller(session->sessionId);
        ret = pimpl_->client.call(caller, UNMAP, request, &response);
        mcResult = convert_syscall_error(ret);
        if (mcResult == MC_DRV_OK) {
            if (s->removeBuffer(sva)) {
                LOG_E("Unmapped buffer not found in session %x: %x",
                      session->sessionId, sva);
            }
        }
    } while (false);

    if (MC_DRV_OK != mcResult) {
        if (MC_DRV_ERR_UNKNOWN == mcResult) {
            mcResult = MC_DRV_ERR_BULK_UNMAPPING;
        }
    }

    EXIT(mcResult);
}

mcResult_t Proxy::mcGetSessionErrorCode(
    mcSessionHandle_t*      session,
    int32_t*                lastErr) {
    ENTER();
    mcResult_t mcResult = MC_DRV_ERR_UNKNOWN;

    do {
        // Call
        GetErrorRequest request;
        request.set_sid(session->sessionId);
        GetErrorResponse response;
        McCaller caller(session->sessionId);
        int ret = pimpl_->client.call(caller, GET_ERROR, request, &response);
        mcResult = convert_syscall_error(ret);
        if (MC_DRV_OK != mcResult) {
            break;
        }

        // Fill in return parameters
        *lastErr = response.exit_code();

    } while (false);

    EXIT(mcResult);
}

mcResult_t Proxy::mcGetMobiCoreVersion(
    uint32_t                /*deviceId*/,
    mcVersionInfo_t*        versionInfo) {
    ENTER();
    mcResult_t mcResult = MC_DRV_ERR_UNKNOWN;

    do {
        // Call
        EmptyMessage request;
        GetVersionResponse response;
        McCaller caller;
        int ret = pimpl_->client.call(caller, GET_VERSION, request, &response);
        if (!ret) {
            ::strncpy(versionInfo->productId, response.product_id().c_str(),
                      MC_PRODUCT_ID_LEN);
            versionInfo->productId[MC_PRODUCT_ID_LEN - 1] = '\0';
            versionInfo->versionMci = response.mci();
            versionInfo->versionSo = response.so();
            versionInfo->versionMclf = response.mclf();
            versionInfo->versionContainer = response.container();
            versionInfo->versionMcConfig = response.mc_config();
            versionInfo->versionTlApi = response.tl_api();
            versionInfo->versionDrApi = response.dr_api();
            if (response.nwd() > PROXY_COPY_MAX_VERSION) {
                versionInfo->versionCmp = PROXY_COPY_MAX_VERSION;
            } else {
                versionInfo->versionCmp = response.nwd();
            }
        }
        mcResult = convert_syscall_error(ret);
    } while (false);

    EXIT(mcResult);
}
