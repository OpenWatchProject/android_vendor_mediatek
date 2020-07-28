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

// Standard
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <memory>
#include <mcSpid.h>
// <t-base
#include "mcimcp.h"             // mclfHeaderV2_t and friends
// Logging
#undef LOG_TAG
#define LOG_TAG "TeeSecureWorld"
#include <log.h>
// Driver
#include "mc_user.h"
#include "mc_admin.h"
// Registry
#include "PrivateRegistry.h"
// Self
#include "SecureWorld.h"

#define MAX_SO_CONT_SIZE 512
#define CRASHDUMP_PATH "/sys/kernel/debug/trustonic_tee/crashdump"

#define MY_LOG_ERRNO(fmt, ...) \
    LOG_E("%s: %s " fmt, __func__, strerror(errno), ##__VA_ARGS__)

struct SecureWorld::Impl {
    int device_fd;
    uint32_t command_id;

    // Thread variables
    pthread_t thread;
    // State of this object
    enum {
        CLOSED,
        OPEN,                   // Device is open
        RUNNING,                // Thread is running
        STOPPED,                // Thread has stopped, device is closed
    } state;
    pthread_cond_t state_cond;
    pthread_mutex_t state_mutex;
    // Block GP TA loading
    bool gp_loading_is_blocked;

    Impl(): device_fd(-1), command_id(0), thread(pthread_self()),
        state(CLOSED), state_cond(PTHREAD_COND_INITIALIZER),
        gp_loading_is_blocked(false) {
        pthread_mutex_init(&state_mutex, NULL);
    }
    void run();
};

SecureWorld::SecureWorld(): pimpl_(new Impl) {}

SecureWorld::~SecureWorld() {
    delete pimpl_;
}

static void* getRootContainer(const struct mc_admin_request*,
                              struct mc_admin_response* response,
                              bool*) {
    void* data = ::malloc(MAX_SO_CONT_SIZE);
    uint32_t length = MAX_SO_CONT_SIZE;

    switch (mcRegistryReadRoot(data, &length)) {
        case MC_DRV_OK:
            response->length = length;
            response->error_no = 0;
            break;
        case MC_DRV_ERR_INVALID_PARAMETER:
            response->error_no = EINVAL;
            break;
        case MC_DRV_ERR_INVALID_DEVICE_FILE:
            response->error_no = ENOENT;
            break;
        default:
            // Some kind of default
            response->error_no = EPERM;
    }

    if (response->error_no) {
        ::free(data);
        return NULL;
    }

    LOG_D("Read root container, size: %u", response->length);
    return data;
}

static void* getSpContainer(const struct mc_admin_request* request,
                            struct mc_admin_response* response,
                            bool*) {
    void* data = ::malloc(MAX_SO_CONT_SIZE);
    uint32_t length = MAX_SO_CONT_SIZE;

    switch (mcRegistryReadSp(request->spid, data, &length)) {
        case MC_DRV_OK:
            response->length = length;
            response->error_no = 0;
            break;
        case MC_DRV_ERR_INVALID_PARAMETER:
            response->error_no = EINVAL;
            break;
        case MC_DRV_ERR_INVALID_DEVICE_FILE:
            response->error_no = ENOENT;
            break;
        default:
            // Some kind of default
            response->error_no = EPERM;
    }

    if (response->error_no) {
        ::free(data);
        return NULL;
    }

    LOG_D("Read SP container for %u, size: %u", request->spid, response->length);
    return data;
}

static void* getTrustletContainer(const struct mc_admin_request* request,
                                  struct mc_admin_response* response,
                                  bool*) {
    void* data = ::malloc(MAX_SO_CONT_SIZE);
    uint32_t length = MAX_SO_CONT_SIZE;

    const mcUuid_t* uuid = reinterpret_cast<const mcUuid_t*>(&request->uuid);
    switch (mcRegistryReadTrustletCon(uuid, request->spid, data, &length)) {
        case MC_DRV_OK:
            response->length = length;
            response->error_no = 0;
            break;
        case MC_DRV_ERR_INVALID_PARAMETER:
            response->error_no = EINVAL;
            break;
        case MC_DRV_ERR_INVALID_DEVICE_FILE:
            response->error_no = ENOENT;
            break;
        case MC_DRV_ERR_OUT_OF_RESOURCES:
            response->error_no = ENOMEM;
            break;
        default:
            // Some kind of default
            response->error_no = EPERM;
    }

    if (response->error_no) {
        ::free(data);
        return NULL;
    }

    LOG_D("Read trustlet container for %u, size: %u", response->spid,
          response->length);
    return data;
}

static void* mapTrustlet(const char* path, uint32_t* length,
                         uint32_t* service_type) {
    int fd = ::open(path, O_RDONLY);
    if (fd < 0) {
        LOG_W("Cannot open trustlet %s (%s)", path, strerror(errno));
        return NULL;
    }

    void* data = NULL;
    *service_type = SERVICE_TYPE_ILLEGAL;
    do {
        struct stat stat;
        if (::fstat(fd, &stat) < 0) {
            MY_LOG_ERRNO("getting size for trustlet %s", path);
            break;
        }

        data = ::mmap(NULL, stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (data == MAP_FAILED) {
            data = NULL;
            MY_LOG_ERRNO("mapping file to memory");
            break;
        }

        /* Give service type to driver so it knows how to allocate and copy */
        mclfHeaderV2_t* header = static_cast<mclfHeaderV2_t*>(data);
        *service_type = header->serviceType;
        *length = static_cast<uint32_t>(stat.st_size);
    } while (false);

    ::close(fd);

    return data;
}

static void* getTrustlet(const struct mc_admin_request* request,
                         struct mc_admin_response* response,
                         bool* is_mmapped) {
    *is_mmapped = true;
    const mcUuid_t* uuid = reinterpret_cast<const mcUuid_t*>(&request->uuid);
    std::string path;

    mcResult_t res = mcRegistryGetTrustletInfo(uuid, request->is_gp,
                     &response->spid, path);
    switch (res) {
        case MC_DRV_OK:
            response->error_no = 0;
            break;
        case MC_DRV_ERR_INVALID_PARAMETER:
            response->error_no = EINVAL;
            break;
        case MC_DRV_ERR_TRUSTLET_NOT_FOUND:
            response->error_no = ENOENT;
            break;
        default:
            // Some kind of default
            response->error_no = EPERM;
    }

    if (response->error_no) {
        return NULL;
    }

    void* data = mapTrustlet(path.c_str(), &response->length,
                             &response->service_type);
    if (!data) {
        response->error_no = errno;
    } else {
        LOG_D("Read spid %u and mmap'd trustlet from %s, total size: %u",
              response->spid, path.c_str(), response->length);
    }
    return data;
}

static void* logCrashDump(const struct mc_admin_request*,
                          struct mc_admin_response* response,
                          bool*) {
    std::ifstream crashdump(CRASHDUMP_PATH);
    if (!crashdump.is_open()) {
        MY_LOG_ERRNO("opening crash dump");
        response->error_no = errno;
        return NULL;
    }

    LOG_E("<t-base halted. Status dump:");
    std::string line;
    while (std::getline(crashdump, line)) {
        LOG_I("%s", line.c_str());
    }

    return NULL;
}

int SecureWorld::setup() {
    pimpl_->device_fd = ::open("/dev/" MC_ADMIN_DEVNODE, O_RDWR);
    if (pimpl_->device_fd < 0) {
        MY_LOG_ERRNO("opening admin device");
        return -1;
    }

    // Request listener role
    uint32_t role = TEE_ROLE_LISTENER;
    if (ioctl(pimpl_->device_fd, MC_ADMIN_IO_REQUEST_ROLE, &role) != 0) {
        LOG_ERRNO("ioctl MC_ADMIN_IO_REQUEST_ROLE");
        ::close(pimpl_->device_fd);
        return -1;
    }

    LOG_D("Check version of <t-base driver");
    struct mc_admin_driver_info info;
    if (::ioctl(pimpl_->device_fd, MC_ADMIN_IO_GET_INFO, &info) != 0) {
        LOG_ERRNO("ioctl MC_ADMIN_IO_GET_INFO");
        ::close(pimpl_->device_fd);
        return -1;
    }

    unsigned int major = info.drv_version >> 16;
    unsigned int minor = info.drv_version & 0xffff;
    LOG_I("driver version: v%d.%d", major, minor);
    if ((major != MCDRVMODULEAPI_VERSION_MAJOR) ||
        (minor != MCDRVMODULEAPI_VERSION_MINOR)) {
        ::close(pimpl_->device_fd);
        errno = EHOSTDOWN;
        _LOG_E("%s in %s (driver version mismatch detected: expected v%d.%d)",
               strerror(errno), __FUNCTION__, MCDRVMODULEAPI_VERSION_MAJOR,
               MCDRVMODULEAPI_VERSION_MINOR);
        return -1;
    }
    pimpl_->command_id = info.initial_cmd_id;

    // We wait 10 times 100ms for the device file to appear
    int counter = 10;
    do {
        int fd = ::open("/dev/" MC_USER_DEVNODE, O_RDWR);
        if (fd >= 0) {
            ::close(fd);
            break;
        }

        if (errno != ENOENT) {
            MY_LOG_ERRNO("opening user device");
            return -1;
        }

        /* Device driver didn't appear yet: give some time */
        usleep(100000);
    } while (counter--);

    if (counter < 0) {
        LOG_E("timed out while waiting for user device to appear");
        return -1;
    }

    pthread_mutex_lock(&pimpl_->state_mutex);
    pimpl_->state = Impl::OPEN;
    pthread_mutex_unlock(&pimpl_->state_mutex);
    LOG_I("TEE is ready");
    return 0;
}

void SecureWorld::close() {
    // In case close is called while waiting is active
    stop();

    pthread_mutex_lock(&pimpl_->state_mutex);
    if (pimpl_->state == Impl::OPEN) {
        ::close(pimpl_->device_fd);
    }
    pimpl_->state = Impl::CLOSED;
    pthread_mutex_unlock(&pimpl_->state_mutex);

    LOG_W("closed");
}

void SecureWorld::receiveSignal(int signum) {
    if ((signum == SIGINT) || (signum == SIGTERM)) {
        stop();
    }
}

int SecureWorld::open() {
    // Create thread
    int ret = 0;
    pthread_mutex_lock(&pimpl_->state_mutex);
    // We don't check the current state here, because we trust the caller
    ret = pthread_create(&pimpl_->thread, NULL, run, pimpl_);
    if (ret) {
        LOG_E("pthread_create failed with error code %d", ret);
        ret =  -1;
    } else {
        if (pthread_setname_np(pimpl_->thread, "McDaemon.SWd")) {
            LOG_W("Error while naming thread");
        }
        pimpl_->state = Impl::RUNNING;
    }
    pthread_mutex_unlock(&pimpl_->state_mutex);
    return ret;
}

void SecureWorld::wait() {
    LOG_D("wait for thread to stop");
    pthread_mutex_lock(&pimpl_->state_mutex);
    while (pimpl_->state == Impl::RUNNING) {
        pthread_cond_wait(&pimpl_->state_cond, &pimpl_->state_mutex);
    }
    pthread_mutex_unlock(&pimpl_->state_mutex);
    LOG_D("thread stopped");
}

void SecureWorld::stop() {
    LOG_D("stop thread");
    pthread_mutex_lock(&pimpl_->state_mutex);
    if (pimpl_->state == Impl::RUNNING) {
        pthread_kill(pimpl_->thread, SIGUSR1);
    }
    pthread_mutex_unlock(&pimpl_->state_mutex);
    pthread_join(pimpl_->thread, NULL);
    LOG_D("thread joined");
}

int SecureWorld::loadDriver(const char* path) {
    struct mc_admin_load_info info;
    uint32_t service_type;

    void* data = mapTrustlet(path, &info.length, &service_type);
    if (!data) {
        return -1;
    }

    LOG_D("Load secure driver %s of size %d", path, info.length);
    info.address = reinterpret_cast<uintptr_t>(data);
    info.spid = 0;

    int ret = ioctl(pimpl_->device_fd, MC_ADMIN_IO_LOAD_DRIVER, &info);
    if (ret != 0) {
        LOG_ERRNO("ioctl MC_ADMIN_IO_LOAD_DRIVER");
    }

    // Free memory occupied by Trustlet data
    ::munmap(data, info.length);
    return ret == 0;
}

int SecureWorld::loadToken(const void* data, size_t length) {
    struct mc_admin_load_info token;

    LOG_D("Load authentication token %p of size %zu", data, length);
    token.address = reinterpret_cast<uintptr_t>(data);
    token.length = static_cast<uint32_t>(length);

    int rc = ioctl(pimpl_->device_fd, MC_ADMIN_IO_LOAD_TOKEN, &token);
    if (rc != 0) {
        LOG_ERRNO("ioctl MC_ADMIN_IO_LOAD_TOKEN");
        return -1;
    }

    return 0;
}

int SecureWorld::loadCheck(mcSpid_t spid, const void* data, size_t length) {
    struct mc_admin_load_info info;

    LOG_D("Load secure object %p, length %zu, spid %x", data, length, spid);
    info.spid = spid;
    info.address = reinterpret_cast<uintptr_t>(data);
    info.length = static_cast<uint32_t>(length);

    int rc = ioctl(pimpl_->device_fd, MC_ADMIN_IO_LOAD_CHECK, &info);
    if (rc != 0) {
        LOG_ERRNO("ioctl MC_ADMIN_IO_LOAD_CHECK");
        return -1;
    }

    return 0;
}

void* SecureWorld::run(void* arg) {
    SecureWorld::Impl* pimpl = static_cast<SecureWorld::Impl*>(arg);
    pimpl->run();
    return NULL;
}

// SIGUSR1 is required to unlock the iotcl function,
// but we have to override the handler with the following dummy function to
// override the default handler : "Term"
static void dummyHandler(int /* sig */) {
    LOG_D("dummyHandler");
    return;
}

void SecureWorld::Impl::run() {
    // Process SIGUSR1 action
    struct sigaction actionSIGUSR1;
    ::memset(&actionSIGUSR1, 0, sizeof(actionSIGUSR1));
    actionSIGUSR1.sa_handler = dummyHandler;
    ::sigemptyset(&actionSIGUSR1.sa_mask);
    // Explicitely specify "no flags" to avoid the reexecution of the
    // ioctl in case of interruption.
    actionSIGUSR1.sa_flags = 0;
    if (::sigaction(SIGUSR1, &actionSIGUSR1, NULL)) {
        LOG_ERRNO("actionSIGUSR1");
        return;
    }
    ::sigaddset(&actionSIGUSR1.sa_mask, SIGUSR1);
    ::pthread_sigmask(SIG_UNBLOCK, &actionSIGUSR1.sa_mask, NULL);

    LOG_I("Start listening to secure world");
    while (true) {
        struct mc_admin_request request;
        int rc;

        rc = ::ioctl(device_fd, MC_ADMIN_IO_GET_DRIVER_REQUEST, &request);
        if (rc) {
            if (errno == EINTR) {
                LOG_D("Giving up on signal");
            } else {
                LOG_ERRNO("Getting request from driver");
            }
            break;
        }

        LOG_D("Request %d received (ID %u)", request.command,
              request.request_id);
        if (command_id != request.request_id) {
            LOG_E("Request ID counters are not synchronised (expected %u, got %u)",
                  command_id, request.request_id);
            break;
        }

        struct mc_admin_response response;
        memset(&response, 0, sizeof(response));
        response.request_id = command_id++;
        void* response_data = NULL;
        bool is_mmapped = false; // Response data needs freeing

        if (gp_loading_is_blocked && request.is_gp) {
            LOG_W("Loading of GP trustlets is currently blocked");
            response.error_no = EAGAIN;
        } else {
            switch (request.command) {
                case MC_DRV_GET_ROOT_CONTAINER:
                    response_data = getRootContainer(&request, &response,
                                                     &is_mmapped);
                    break;
                case MC_DRV_GET_SP_CONTAINER:
                    response_data = getSpContainer(&request, &response,
                                                   &is_mmapped);
                    break;
                case MC_DRV_GET_TRUSTLET_CONTAINER:
                    response_data = getTrustletContainer(&request, &response,
                                                         &is_mmapped);
                    break;
                case MC_DRV_GET_TRUSTLET:
                    response_data = getTrustlet(&request, &response,
                                                &is_mmapped);
                    break;
                case MC_DRV_SIGNAL_CRASH:
                    logCrashDump(&request, &response, &is_mmapped);
                    break;
                default:
                    LOG_E("Unknown command");
                    response.error_no = EBADRQC;
            }
        }

        ssize_t ret = ::write(device_fd, &response, sizeof(response));
        if (ret != sizeof(response)) {
            LOG_ERRNO("Sending response to driver");
            ret = -1;
        } else if (response.length > 0) {
            ssize_t expected_length = response.length;
            ret = ::write(device_fd, response_data, response.length);
            if (ret != expected_length) {
                LOG_ERRNO("Sending response data to driver");
                ret = -1;
            }
        }

        if (response_data && is_mmapped) {
            ::munmap(response_data, response.length);
        } else {
            ::free(response_data);
        }
    }

    // An error has occured, signal the ending of the thread
    LOG_I("Stop listening to secure world");
    pthread_mutex_lock(&state_mutex);
    state = STOPPED;
    ::close(device_fd);
    pthread_cond_broadcast(&state_cond);
    pthread_mutex_unlock(&state_mutex);
    LOG_D("end thread");
}

void SecureWorld::blockGpLoading() {
    pimpl_->gp_loading_is_blocked = true;
    LOG_I("GP loading blocked");
}

void SecureWorld::unblockGpLoading() {
    pimpl_->gp_loading_is_blocked = false;
    LOG_I("GP loading unblocked");
}
