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

#include <vector>

#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>

#include <tee_client_api.h>     // TEEC_UUID
#include "GpTci.h"              // _TEEC_TCI

#undef LOG_TAG
#define LOG_TAG "TeeSyDriverClient"
#include "log.h"
#include "driver_client.h"

struct DriverClient::Impl {
    struct GpSession {
        uint32_t id;
        _TEEC_TCI* tci;
        GpSession(uint32_t i, uint64_t t): id(i) {
            tci = reinterpret_cast<_TEEC_TCI*>(static_cast<uintptr_t>(t));
        }
    };
    pthread_mutex_t gp_sessions_mutex;
    std::vector<GpSession*> gp_sessions;
    int fd;
    Impl(): fd(-1) {
        ::pthread_mutex_init(&gp_sessions_mutex, NULL);
    }
    ~Impl() {
        ::pthread_mutex_destroy(&gp_sessions_mutex);
    }
};

DriverClient::DriverClient(): pimpl_(new Impl) {
}

DriverClient::~DriverClient() {
    if (isOpen()) {
        close();
    }
    delete pimpl_;
}

int DriverClient::open() {
    int fd = ::open("/dev/" MC_USER_DEVNODE, O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        _LOG_E("%s in %s", strerror(errno), __FUNCTION__);
        return -1;
    }

    struct mc_version_info version_info;
    if (::ioctl(fd, MC_IO_VERSION, &version_info) < 0) {
        _LOG_E("%s in %s", strerror(errno), __FUNCTION__);
        (void)::close(fd);
        return -1;
    }

    unsigned int major = version_info.version_nwd >> 16;
    unsigned int minor = version_info.version_nwd & 0xffff;
    LOG_I("driver version: v%d.%d", major, minor);
    if ((major != MCDRVMODULEAPI_VERSION_MAJOR) ||
        (minor != MCDRVMODULEAPI_VERSION_MINOR)) {
        (void)::close(fd);
        errno = EHOSTDOWN;
        _LOG_E("%s in %s (driver version mismatch detected: expected v%d.%d)",
               strerror(errno), __FUNCTION__, MCDRVMODULEAPI_VERSION_MAJOR,
               MCDRVMODULEAPI_VERSION_MINOR);
        return -1;
    }

    pimpl_->fd = fd;
    LOG_I("driver client open");
    return 0;
}

int DriverClient::close() {
    int ret = ::close(pimpl_->fd);
    if (ret) {
        _LOG_E("%s in %s", strerror(errno), __FUNCTION__);
    }
    pimpl_->fd = -1;
    LOG_I("driver client closed");
    return ret;
}

bool DriverClient::isOpen() const {
    return pimpl_->fd != -1;
}

int DriverClient::hasOpenSessions() const {
    int ret = ::ioctl(pimpl_->fd, MC_IO_HAS_SESSIONS);
    if (ret) {
        _LOG_E("%s in %s", strerror(errno), __FUNCTION__);
    }
    return ret;
}

int DriverClient::openSession(struct mc_ioctl_open_session& session) {
    int ret = ::ioctl(pimpl_->fd, MC_IO_OPEN_SESSION, &session);
    if (ret) {
        _LOG_E("%s in %s", strerror(errno), __FUNCTION__);
        return ret;
    }
    if (session.is_gp_uuid) {
        pthread_mutex_lock(&pimpl_->gp_sessions_mutex);
        pimpl_->gp_sessions.push_back(
            new Impl::GpSession(session.sid, session.tci));
        pthread_mutex_unlock(&pimpl_->gp_sessions_mutex);
    }
    LOG_D("session %x open", session.sid);
    return ret;
}

int DriverClient::openTrustlet(struct mc_ioctl_open_trustlet& trustlet) {
    int ret = ::ioctl(pimpl_->fd, MC_IO_OPEN_TRUSTLET, &trustlet);
    if (ret) {
        _LOG_E("%s in %s", strerror(errno), __FUNCTION__);
        return ret;
    }
    LOG_D("session %x open", trustlet.sid);
    return ret;
}

int DriverClient::closeSession(uint32_t session_id) {
    LOG_D("session %x close", session_id);
    int ret = ::ioctl(pimpl_->fd, MC_IO_CLOSE_SESSION, session_id);
    if (ret) {
        _LOG_E("%s in %s", strerror(errno), __FUNCTION__);
        return ret;
    }
    pthread_mutex_lock(&pimpl_->gp_sessions_mutex);
    for (auto it = pimpl_->gp_sessions.begin();
            it != pimpl_->gp_sessions.end(); it++) {
        auto gp_session = *it;
        if (gp_session->id == session_id) {
            pimpl_->gp_sessions.erase(it);
            delete gp_session;
            break;
        }
    }
    pthread_mutex_unlock(&pimpl_->gp_sessions_mutex);
    LOG_D("session %x closed", session_id);
    return ret;
}

int DriverClient::notify(uint32_t session_id) {
    int ret = ::ioctl(pimpl_->fd, MC_IO_NOTIFY, session_id);
    if (ret) {
        _LOG_E("%s in %s", strerror(errno), __FUNCTION__);
        return ret;
    }
    LOG_D("session %x notification sent", session_id);
    return ret;
}

int DriverClient::waitNotification(const struct mc_ioctl_wait& wait) {
    LOG_D("session %x wait for notification", wait.sid);
    int ret = ::ioctl(pimpl_->fd, MC_IO_WAIT, &wait);
    if (ret && ((errno != ETIME) || !wait.partial)) {
        _LOG_E("%s in %s", strerror(errno), __FUNCTION__);
        return ret;
    }
    LOG_D("session %x notification received", wait.sid);
    return ret;
}

int DriverClient::malloc(uint8_t** buffer, uint32_t length) {
    *buffer = static_cast<uint8_t*>(mmap(0, length, PROT_READ | PROT_WRITE,
                                         MAP_SHARED, pimpl_->fd, 0));
    if (*buffer == MAP_FAILED) {
        _LOG_E("%s in %s", strerror(errno), __FUNCTION__);
        return -1;
    }
    return 0;
}

int DriverClient::free(uint8_t* buffer, uint32_t length) {
    int ret = munmap(buffer, length);
    if (ret) {
        _LOG_E("%s in %s", strerror(errno), __FUNCTION__);
    }
    return ret;
}

int DriverClient::map(struct mc_ioctl_map& map) {
    int ret = ::ioctl(pimpl_->fd, MC_IO_MAP, &map);
    if (ret) {
        _LOG_E("%s in %s", strerror(errno), __FUNCTION__);
        return ret;
    }
    LOG_D("session %x buffer(s) mapped", map.sid);
    return ret;
}

int DriverClient::unmap(const struct mc_ioctl_map& map) {
    int ret = ::ioctl(pimpl_->fd, MC_IO_UNMAP, &map);
    if (ret) {
        _LOG_E("%s in %s", strerror(errno), __FUNCTION__);
        return ret;
    }
    LOG_D("session %x buffer(s) unmapped", map.sid);
    return ret;
}

int DriverClient::getError(struct mc_ioctl_geterr& err) {
    int ret = ::ioctl(pimpl_->fd, MC_IO_ERR, &err);
    if (ret) {
        _LOG_E("%s in %s", strerror(errno), __FUNCTION__);
        return ret;
    }
    LOG_D("session %x exit code %d", err.sid, err.value);
    return ret;
}

int DriverClient::getVersion(struct mc_version_info& version_info) {
    int ret = ::ioctl(pimpl_->fd, MC_IO_VERSION, &version_info);
    if (ret) {
        _LOG_E("%s in %s", strerror(errno), __FUNCTION__);
    }
    return ret;
}

// This class will need to handle the GP protocol at some point, but for now it
// only deals with cancellation
int DriverClient::gpRequestCancellation(uint32_t session_id) {
    bool found = false;
    pthread_mutex_lock(&pimpl_->gp_sessions_mutex);
    for (auto it = pimpl_->gp_sessions.begin(); it != pimpl_->gp_sessions.end();
            it++) {
        auto gp_session = *it;
        if (gp_session->id == session_id) {
            // Will be reset by caller at next InvokeCommand
            gp_session->tci->operation.isCancelled = true;
            found = true;
        }
    }
    pthread_mutex_unlock(&pimpl_->gp_sessions_mutex);
    if (!found) {
        errno = ENOENT;
        return -1;
    }
    return notify(session_id);
}

int DriverClient::checkAdminPid(pid_t pid) {
    struct mc_authenticator_check info;
    info.pid = pid;
    int ret = ::ioctl(pimpl_->fd, MC_IO_AUTHENTICATOR_CHECK, &info);
    if (ret) {
        _LOG_E("%s in %s", strerror(errno), __FUNCTION__);
    }
    return ret;
}
