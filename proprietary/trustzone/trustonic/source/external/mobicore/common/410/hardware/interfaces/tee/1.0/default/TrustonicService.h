/*
 * Copyright (c) 2019 TRUSTONIC LIMITED
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

#ifndef VENDOR_TRUSTONIC_TEE_V1_0_VENDOR_SERVICE_H
#define VENDOR_TRUSTONIC_TEE_V1_0_VENDOR_SERVICE_H

#include <vendor/trustonic/tee/1.0/ITee.h>
#include <vendor/trustonic/tee/tui/1.0/ITui.h>
#include <vendor/trustonic/tee/1.0/ITeeCallback.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

using vendor::trustonic::tee::V1_0::ITee;
using vendor::trustonic::tee::V1_0::ITeeCallback;
using vendor::trustonic::tee::tui::V1_0::ITui;

namespace vendor {
namespace trustonic {

class TrustonicService {
private:
    struct Impl;
    const std::unique_ptr<Impl> pimpl_;

public:
    TrustonicService();
    ~TrustonicService();

    android::sp<::ITee> getTee();
    android::sp<::ITui> getTui();
    void setTeeCallback(const android::sp<ITeeCallback> &callback);
    android::sp<ITeeCallback> getTeeCallback();

};

}  // namespace trustonic
}  // namespace vendor

#endif  // VENDOR_TRUSTONIC_TEE_V1_0_VENDOR_SERVICE_H
