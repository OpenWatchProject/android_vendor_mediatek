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

#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <dirent.h>
#include <stdlib.h>

#undef LOG_TAG
#define LOG_TAG "TeeEndorsementInstaller"
#include <log.h>
#include "FSD2.h"
#include "EndorsementInstaller.h"

struct EndorsementInstaller::Impl {
    RegistryServer& registry_server;
    const std::string& registry_path;
    std::mutex          mutex;
    std::thread         thread;
    bool                keep_running = false;
    Impl(RegistryServer& rs, const std::string& rp):
        registry_server(rs), registry_path(rp) {}

    void run();
};

EndorsementInstaller::EndorsementInstaller(
    RegistryServer& registry_server,
    const std::string& registry_path):
    pimpl_(new Impl(registry_server, registry_path)) {}

EndorsementInstaller::~EndorsementInstaller() {
    delete pimpl_;
}

int EndorsementInstaller::open() {
    // Create FileSystem open thread
    pimpl_->keep_running = true;
    pimpl_->thread = std::thread(&Impl::run, pimpl_);
    return 0;
}

void EndorsementInstaller::close() {
    // Stop thread
    {
        std::lock_guard<std::mutex> lock(pimpl_->mutex);
        pimpl_->keep_running = false;
    }
    pimpl_->thread.join();
}

void EndorsementInstaller::Impl::run() {
    ::pthread_setname_np(pthread_self(), "McDaemon.ENIN");
    LOG_I("Starting endorsement installer thread");
    // The registry is created before we are started, in the 'right' place, so
    // let's wait for it.
    while (::access(registry_path.c_str(), F_OK) < 0) {
        LOG_D("Wait a bit longer for registry to appear");
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (!keep_running) {
                LOG_I("Abort endorsement installer thread");
                return;
            }
        }
        sleep(1);
    }
    uint32_t message_print_countdown = 0;
    // Tokens could be created during production or OTA
    while (registry_server.installEndorsementToken()) {
        message_print_countdown ++;
        /* Print a message every minute */
        if (message_print_countdown == 60) {
            LOG_W("Endorsement token not found, wait a bit longer");
            message_print_countdown = 0;
        }
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (!keep_running) {
                LOG_I("Abort endorsement installer thread");
                return;
            }
        }
        sleep(1);
    }

    LOG_I("Exiting endorsement installer thread");
}
