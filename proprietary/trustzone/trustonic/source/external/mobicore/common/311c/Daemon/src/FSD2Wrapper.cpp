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

#include <string>
#include <vector>

#include <dirent.h>
#include <pthread.h>
#include <stdlib.h>

#undef LOG_TAG
#define LOG_TAG "TeeFileSystemWrapper"
#include <log.h>
#include "FSD2Wrapper.h"
#include "FSD2.h"

struct FileSystemWrapper::Impl {
    RegistryServer& registry_server;
    const std::vector<std::string>& registry_paths;
    UpgradeStorage&     upgrade_storage;
    FileSystem          file_system;
    pthread_mutex_t     mutex;
    pthread_t           thread;
    bool                keep_running;
    bool                is_fs_open;
    bool                is_us_open;
    Impl(RegistryServer& rs, const std::vector<std::string>& rp,
         UpgradeStorage& us):
        registry_server(rs), registry_paths(rp), upgrade_storage(us),
        thread(pthread_self()), keep_running(false), is_fs_open(false),
        is_us_open(false) {
        pthread_mutex_init(&mutex, NULL);
    }

    void run();
};

FileSystemWrapper::FileSystemWrapper(
    RegistryServer& registry_server,
    const std::vector<std::string>& registry_paths,
    UpgradeStorage& upgrade_storage):
    pimpl_(new Impl(registry_server, registry_paths, upgrade_storage)) {}

FileSystemWrapper::~FileSystemWrapper() {
    delete pimpl_;
}

int FileSystemWrapper::open() {
    // Create FileSystem open thread
    pimpl_->keep_running = true;
    pimpl_->is_fs_open = false;
    pimpl_->is_us_open = false;
    int ret = pthread_create(&pimpl_->thread, NULL, run, pimpl_);
    if (ret) {
        LOG_E("pthread_create failed with error code %d", ret);
        return -1;
    }
    ret = pthread_setname_np(pimpl_->thread, "McDaemon.FSDW"); // Open
    if (ret) {
        LOG_W("Error while naming FSD2 thread");
    }
    return 0;
}

void FileSystemWrapper::close() {
    // Stop thread
    pthread_mutex_lock(&pimpl_->mutex);
    pimpl_->keep_running = false;
    pthread_mutex_unlock(&pimpl_->mutex);
    pthread_join(pimpl_->thread, NULL);
    if (pimpl_->is_fs_open) {
        if (pimpl_->is_us_open) {
            pimpl_->upgrade_storage.close();
        }
        pimpl_->file_system.close();
    }
}

void* FileSystemWrapper::run(void* arg) {
    FileSystemWrapper::Impl* pimpl = static_cast<FileSystemWrapper::Impl*>(arg);
    pimpl->run();
    return NULL;
}

void FileSystemWrapper::Impl::run() {
    LOG_I("Starting FileSystem open thread");
    pthread_mutex_lock(&mutex);
    while (keep_running) {
        // The registry is created before we are started, in the 'right' place, so
        // let's wait for it.
        while (::access(registry_paths[0].c_str(), F_OK) < 0) {
            LOG_D("Wait a bit longer for registry to appear");
            sleep(1);
        }

        // Check whether we need to look for an Android pre-N registry
        std::string marker_path = registry_paths[0] + "/.migration-done";
        if (::access(marker_path.c_str(), F_OK) < 0) {
            // Just assume that the marker is missing
            std::string old_registry_dir("/data/misc/mcRegistry");
            // If we cannot access the old registry, we skip
            if (::access(old_registry_dir.c_str(), F_OK) == 0) {
                LOG_I("Old registry found, move contents");
                DIR* dir = ::opendir(old_registry_dir.c_str());
                if (!dir) {
                    LOG_ERRNO("(ignored) open old registry dir");
                } else {
                    struct dirent* dirent;
                    while ((dirent = ::readdir(dir))) {
                        if ((::strcmp(dirent->d_name, ".") == 0) ||
                                (::strcmp(dirent->d_name, "..") == 0)) {
                            continue;
                        }
                        // Need to rename from the old registry name to the new
                        std::string old_path(old_registry_dir + "/" + dirent->d_name);
                        std::string new_path(registry_paths[0] + "/" + dirent->d_name);
                        LOG_I("  %s -> %s", old_path.c_str(), new_path.c_str());
                        if (::rename(old_path.c_str(), new_path.c_str())) {
                            LOG_ERRNO("registry file rename");
                            _LOG_E("*** ABORT TRUSTONIC DAEMON ***");
                            sleep(1);
                            exit(1);
                        }
                    }
                    ::closedir(dir);
                }
            }
            // All done, set marker
            FILE* f_marker = ::fopen(marker_path.c_str(), "w");
            if (f_marker) {
                ::fclose(f_marker);
            }
        } else {
            LOG_D("Registry marker found, no need to move any files");
        }

        // Re-install, as new ones may have appeared with the registry
        registry_server.installEndorsementToken();

        int rc = file_system.open();
        if (!rc) {
            LOG_I("FileSystem open");
            if (!upgrade_storage.open()) {
                LOG_I("FileSystem update started");
                is_us_open = true;
            }
            break;
        }
        pthread_mutex_unlock(&mutex);
        LOG_I("FileSystem open failed, re-try...");
        sleep(1);
        pthread_mutex_lock(&mutex);
    }
    pthread_mutex_unlock(&mutex);
    LOG_I("Exiting filesystem open thread");
}
