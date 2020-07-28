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

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>

#undef LOG_TAG
#define LOG_TAG "TeeSyAuthServer"
#include "log.h"
// Driver
#include "mc_user.h"
#include "mc_admin.h"
// Proxy
#include "proxy_common.h"
// Self
#include "auth_server.h"

using namespace com::trustonic::tee_proxy;

class Device {
    int fd_ = -1;
public:
    ~Device() {
        ::close(fd_);
    }
    int open() {
        fd_ = ::open("/dev/" MC_ADMIN_DEVNODE, O_RDWR);
        if (fd_ < 0) {
            LOG_ERRNO("opening admin device");
            return -1;
        }

        // Request authenticator role
        uint32_t role = TEE_ROLE_AUTHENTICATOR;
        if (::ioctl(fd_, MC_ADMIN_IO_REQUEST_ROLE, &role) != 0) {
            LOG_ERRNO("ioctl MC_ADMIN_IO_REQUEST_ROLE");
            ::close(fd_);
            return -1;
        }

        return 0;
    }
};

int AuthServer::sendList(const std::list<UIDInfo>& uid_infos) {
    // Connect to driver
    Device device;
    if (device.open() < 0) {
        return -1;
    }

    // Connect to proxy
    CommonConnection conn;
    if (conn.connect() < 0) {
        return -1;
    }

    // Declare self as authenticator and send list of authorised UIDs
    AuthentifierSendListRequest request;
    for (auto& uid_info: uid_infos) {
        auto app_info = request.add_app_info();
        app_info->set_uid(uid_info.uid);
        app_info->set_validated(uid_info.validated);
    }
    // Serialize request
    std::string buffer;
    if (!request.SerializeToString(&buffer)) {
        LOG_E("Failed to serialize");
        return -1;
    }
    if (conn.sendRequest(AUTHENTIFIER_SEND_LIST, buffer)) {
        return -1;
    }
    int method_rc;
    if (conn.receiveResponse(&buffer, &method_rc)) {
        return -1;
    }
    if (method_rc) {
        errno = method_rc;
        LOG_ERRNO("call");
        return -1;
    }

    LOG_I("Successfully sent list of UIDs to proxy server");
    return 0;
}
