/*
 * Copyright (c) 2013-2017 TRUSTONIC LIMITED
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

#ifndef VENDOR_TRUSTONIC_TEECLIENT_V1_0_TUIMANAGER_H
#define VENDOR_TRUSTONIC_TEECLIENT_V1_0_TUIMANAGER_H

#include <mutex>
#include <thread>

#include <utils/Log.h>
#include <vendor/trustonic/tee/tui/1.0/ITuiCallback.h>
#include <vendor/trustonic/tee/1.0/ITeeCallback.h>

#include "tui_ioctl.h"

namespace vendor {
namespace trustonic {
namespace tee {
namespace tui {
namespace V1_0 {
namespace implementation {

class TuiManager {

    struct Impl;
    const std::unique_ptr<Impl> pimpl_;

public:
    TuiManager();
    ~TuiManager();

    void registerTuiCallback(const ::android::sp<::vendor::trustonic::tee::tui::V1_0::ITuiCallback>& callback);
    void registerTeeCallback(const ::android::sp<::vendor::trustonic::tee::V1_0::ITeeCallback>& callback);
    uint32_t notifyReeEvent(uint32_t eventType);
    uint32_t notifyScreenSizeUpdate(uint32_t width, uint32_t height);
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace tui
}  // namespace tee
}  // namespace trustonic
}  // namespace vendor

#endif  // VENDOR_TRUSTONIC_TEECLIENT_V1_0_TEECLIENT_H
