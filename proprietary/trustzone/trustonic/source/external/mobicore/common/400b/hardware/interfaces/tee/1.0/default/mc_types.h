/*
 * Copyright (c) 2018 TRUSTONIC LIMITED
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

#ifndef MC_TYPES_H
#define MC_TYPES_H

#include <algorithm>
#include <memory>
#include <mutex>
#include <vector>

#include "common_types.h"

namespace vendor {
namespace trustonic {

class McSession: public Session {
    uint32_t id_ = 0;
public:
    McSession(
            const std::shared_ptr<Service>& service):
        Session(service) {}

    void setId(
            uint32_t id) {
        id_ = id;
    }

    uint32_t id() const {
        return id_;
    }
};

class McClient: public Client {
    std::mutex sessions_mutex_;
    std::vector<std::shared_ptr<McSession>> sessions_;
    int pid_ = 0;
public:
    McClient(
            const std::shared_ptr<Service>& service,
            int pid = 0):
        Client(service), pid_(pid) {}

    int pid() const {
        return pid_;
    }

    void addSession(
            const std::shared_ptr<McSession>& session) {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        sessions_.emplace_back(session);
        ALOGM("%zu sessions", sessions_.size());
    }

    bool removeSession(
            uint32_t id) {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        auto it = std::find_if(sessions_.begin(), sessions_.end(), [id]
                               (const auto& session) {
            return session->id() == id;
        }
        );
        if (it == sessions_.end()) {
            return false;
        }

        sessions_.erase(it);
        ALOGM("%zu sessions", sessions_.size());
        return true;
    }

    std::shared_ptr<McSession> getSession(
            uint32_t id) {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        auto it = std::find_if(sessions_.begin(), sessions_.end(), [id]
                               (const auto& session) {
            return session->id() == id;
        }
        );
        if (it == sessions_.end()) {
            return nullptr;
        }

        return *it;
    }

    void flush() override {
        Client::flush();
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        for (auto& session: sessions_) {
            session->flush();
        }

        sessions_.clear();
        ALOGM("%zu sessions", sessions_.size());
    }
};

class McManager {
    std::mutex devices_mutex_;
    std::vector<std::shared_ptr<McClient>> devices_;

    auto findDeviceUnlocked(
            int pid) {
        return std::find_if(devices_.begin(), devices_.end(), [pid]
                            (const auto& device) {
            return device->pid() == pid;
        }
        );
    }
public:
    std::shared_ptr<McClient> addDevice(
            const std::shared_ptr<Service>& service,
            int pid) {
        std::lock_guard<std::mutex> lock(devices_mutex_);
        auto device = std::make_shared<McClient>(service, pid);
        devices_.emplace_back(device);
        ALOGM("%zu devices", devices_.size());
        return device;
    }

    bool removeDevice(
            int pid) {
        std::lock_guard<std::mutex> lock(devices_mutex_);
        auto it = findDeviceUnlocked(pid);
        if (it != devices_.end()) {
            auto& device = *it;
            device->flush();
            devices_.erase(it);
            ALOGM("%zu devices", devices_.size());
            return true;
        }

        return false;
    }

    std::shared_ptr<McClient> getDevice(
            int pid) {
        std::lock_guard<std::mutex> lock(devices_mutex_);
        auto it = findDeviceUnlocked(pid);
        if (it == devices_.end()) {
            return nullptr;
        }

        return *it;
    }

    std::shared_ptr<McSession> getSession(
            int pid,
            uint32_t id,
            mcResult_t* mc_result) {
        auto device = getDevice(pid);
        if (!device) {
            ALOGE("No device.");
            *mc_result = MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN;
            return nullptr;
        }

        auto session = device->getSession(id);
        if (!session) {
            ALOGE("Failed to find requested session.");
            *mc_result = MC_DRV_ERR_UNKNOWN_SESSION;
            return nullptr;
        }

        return session;
    }

    void flush() {
        ALOGH("%s", __PRETTY_FUNCTION__);
        std::lock_guard<std::mutex> lock(devices_mutex_);
        for (auto& device: devices_) {
            device->flush();
        }

        devices_.clear();
        ALOGM("%zu devices", devices_.size());
    }

    void flushPid(int pid) {
        ALOGH("%s pid %d", __PRETTY_FUNCTION__, pid);
        std::lock_guard<std::mutex> lock(devices_mutex_);
        auto it = findDeviceUnlocked(pid);
        if (it == devices_.end()) {
            return;
        }

        auto device = *it;
        device->flush();
        devices_.erase(it);
        ALOGM("%zu devices", devices_.size());
    }
};

}
}

#endif // MC_TYPES_H
