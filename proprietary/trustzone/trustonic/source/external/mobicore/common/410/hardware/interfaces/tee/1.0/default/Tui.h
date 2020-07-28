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

#ifndef VENDOR_TRUSTONIC_TEE_TUI_V1_0_TUI_H
#define VENDOR_TRUSTONIC_TEE_TUI_V1_0_TUI_H

#include <vendor/trustonic/tee/tui/1.0/ITui.h>
#include <vendor/trustonic/tee/1.0/ITeeCallback.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include "TrustonicService.h"

namespace vendor {
namespace trustonic {
namespace tee {
namespace tui {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct Tui : public ITui {
    Tui(std::shared_ptr<TrustonicService> service_);
    ~Tui() override;

    // Methods from ::vendor::trustonic::tee::tui::V1_0::ITui follow.
    Return<uint32_t> notifyReeEvent(uint32_t code) override;
    Return<void> registerTuiCallback(const sp<::vendor::trustonic::tee::tui::V1_0::ITuiCallback>& callback) override;
    Return<uint32_t> notifyScreenSizeUpdate(uint32_t width, uint32_t height) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

private:
    struct Impl;
    const std::unique_ptr<Impl> pimpl_;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace tui
}  // namespace tee
}  // namespace trustonic
}  // namespace vendor

#endif  // VENDOR_TRUSTONIC_TEE_TUI_V1_0_TUI_H
