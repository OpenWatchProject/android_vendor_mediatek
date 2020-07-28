/*
 * Copyright (c) 2013-2015 TRUSTONIC LIMITED
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

#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#undef LOG_TAG
#define LOG_TAG "TeeSyCommonClient"
#include "log.h"
#include "driver_client.h"
#ifndef WITHOUT_PROXY
#include "proxy_client.h"
#endif
#include "common_client.h"

#ifdef LOG_FPRINTF
// Set default log destination (needs to be somewhere)
FILE* mc_log_file_ = stdout;
#endif

struct CommonClient::Impl {
    pthread_mutex_t mutex;
    int open_count;
    DriverClient driver;
#ifndef WITHOUT_PROXY
    ProxyClient proxy;
#endif
    IClient* client;
    OpenMode open_mode;
    Impl(): open_count(0), client(NULL), open_mode(AUTO) {
        pthread_mutex_init(&mutex, NULL);
    }
};

CommonClient::CommonClient(): pimpl_(new Impl) {
}

CommonClient::~CommonClient() {
    delete pimpl_;
}

int CommonClient::open() {
    int ret = 0;
    pthread_mutex_lock(&pimpl_->mutex);
    if (pimpl_->client && !pimpl_->client->isOpen()) {
        pimpl_->open_count = 0;
    }
    if (pimpl_->open_count == 0) {
        if ((pimpl_->open_mode != PROXY) && (pimpl_->driver.open() >= 0)) {
            pimpl_->client = &pimpl_->driver;
#ifndef WITHOUT_PROXY
        } else if ((pimpl_->open_mode != DRIVER) &&
                   (pimpl_->proxy.open() >= 0)) {
            pimpl_->client = &pimpl_->proxy;
#endif
        } else {
            LOG_E("Failed to open lower layers: %s", strerror(errno));
            ret = -1;
        }
    }
    if (pimpl_->client) {
        pimpl_->open_count++;
    }
    pthread_mutex_unlock(&pimpl_->mutex);
    LOG_D("%s ret=%d open_count=%d", __FUNCTION__, ret, pimpl_->open_count);
    return ret;
}

int CommonClient::closeCheck() {
    int ret;
    pthread_mutex_lock(&pimpl_->mutex);
    if (pimpl_->open_count > 1) {
        pimpl_->open_count--;
        ret = 0;
    } else {
        errno = EPERM;
        ret = -1;
    }
    pthread_mutex_unlock(&pimpl_->mutex);
    LOG_D("%s ret=%d open_count=%d", __FUNCTION__, ret, pimpl_->open_count);
    return ret;
}

int CommonClient::close() {
    int ret = -1;
    pthread_mutex_lock(&pimpl_->mutex);
    /* Not open */
    if (!pimpl_->client) {
        errno = EBADF;
    } else {
        /* Last token */
        if (pimpl_->open_count == 1) {
            ret = pimpl_->client->close();
            pimpl_->client = NULL;
        } else {
            ret = 0;
        }
        pimpl_->open_count--;
    }
    pthread_mutex_unlock(&pimpl_->mutex);
    LOG_D("%s ret=%d open_count=%d", __FUNCTION__, ret, pimpl_->open_count);
    return ret;
}

bool CommonClient::isOpen() const {
    return pimpl_->open_count > 0;
}

int CommonClient::hasOpenSessions() const {
    if (!pimpl_->client) {
        errno = EBADF;
        return -1;
    }
    return pimpl_->client->hasOpenSessions();
}

int CommonClient::openSession(struct mc_ioctl_open_session& session) {
    if (!pimpl_->client) {
        errno = EBADF;
        return -1;
    }
    return pimpl_->client->openSession(session);
}

int CommonClient::openTrustlet(struct mc_ioctl_open_trustlet& trustlet) {
    if (!pimpl_->client) {
        errno = EBADF;
        return -1;
    }
    return pimpl_->client->openTrustlet(trustlet);
}

int CommonClient::closeSession(uint32_t session_id) {
    if (!pimpl_->client) {
        errno = EBADF;
        return -1;
    }
    return pimpl_->client->closeSession(session_id);
}

int CommonClient::notify(uint32_t session_id) {
    if (!pimpl_->client) {
        errno = EBADF;
        return -1;
    }
    return pimpl_->client->notify(session_id);
}

int CommonClient::waitNotification(const struct mc_ioctl_wait& wait) {
    if (!pimpl_->client) {
        errno = EBADF;
        return -1;
    }
    return pimpl_->client->waitNotification(wait);
}

int CommonClient::malloc(uint8_t** buffer, uint32_t length) {
    // Check length here to make sure we are consistent, with or without proxy
    if ((length == 0) || (length > BUFFER_LENGTH_MAX)) {
        errno = EINVAL;
        return -1;
    }
    if (!pimpl_->client) {
        errno = EBADF;
        return -1;
    }
    return pimpl_->client->malloc(buffer, length);
}

int CommonClient::free(uint8_t* buffer, uint32_t length) {
    if (!pimpl_->client) {
        errno = EBADF;
        return -1;
    }
    return pimpl_->client->free(buffer, length);
}

int CommonClient::map(struct mc_ioctl_map& map) {
    if (!pimpl_->client) {
        errno = EBADF;
        return -1;
    }
    return pimpl_->client->map(map);
}

int CommonClient::unmap(const struct mc_ioctl_map& map) {
    if (!pimpl_->client) {
        errno = EBADF;
        return -1;
    }
    return pimpl_->client->unmap(map);
}

int CommonClient::getError(struct mc_ioctl_geterr& err) {
    if (!pimpl_->client) {
        errno = EBADF;
        return -1;
    }
    return pimpl_->client->getError(err);
}

int CommonClient::getVersion(struct mc_version_info& version_info) {
    if (!pimpl_->client) {
        errno = EBADF;
        return -1;
    }
    return pimpl_->client->getVersion(version_info);
}

int CommonClient::gpRequestCancellation(uint32_t session_id) {
    if (!pimpl_->client) {
        errno = EBADF;
        return -1;
    }
    return pimpl_->client->gpRequestCancellation(session_id);
}

void CommonClient::setOpenMode(OpenMode open_mode) {
#ifdef WITHOUT_PROXY
    (void) open_mode;
#else
    pimpl_->open_mode = open_mode;
#endif
}
