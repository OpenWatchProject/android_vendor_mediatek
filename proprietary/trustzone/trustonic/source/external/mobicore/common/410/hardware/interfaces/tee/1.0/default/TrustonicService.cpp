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
#define LOG_TAG "vendor.trustonic.tee@1.0-service"

//#define LOG_NDEBUG 0
//#define TEESERVICE_DEBUG
//#define TEESERVICE_MEMLEAK_CHECK

#include <mutex>
#include <vector>

#include <utils/Log.h>
#include "TrustonicService.h"
#include "Tee.h"
#include "Tui.h"
#include "driver.h"
#include "gp_types.h"
#include "mc_types.h"

using vendor::trustonic::tee::V1_0::ITee;
using vendor::trustonic::tee::tui::V1_0::ITui;
using vendor::trustonic::tee::V1_0::implementation::Tee;
using vendor::trustonic::tee::tui::V1_0::implementation::Tui;

namespace vendor {
namespace trustonic {

struct TrustonicService::Impl {

    android::sp<::ITui> tui;
    android::sp<::ITee> tee;
    android::sp<::vendor::trustonic::tee::V1_0::ITeeCallback> teecallback;
};

// Using new here because std::make_unique is not part of C++11
TrustonicService::TrustonicService(): pimpl_(new Impl) {
    std::shared_ptr<TrustonicService> ptr(this);
    pimpl_->tui = new Tui(ptr);
    pimpl_->tee = new Tee(ptr);
    ALOGH("%s tee and tui created.", __func__);
}

TrustonicService::~TrustonicService() {
}

android::sp<::ITee> TrustonicService::getTee() {
    return pimpl_->tee;
}

android::sp<::ITui> TrustonicService::getTui() {
    return pimpl_->tui;
}

void TrustonicService::setTeeCallback(const android::sp<ITeeCallback> &callback) {
    pimpl_->teecallback = callback;
}

android::sp<::ITeeCallback> TrustonicService::getTeeCallback() {
    return pimpl_->teecallback;
}

}  // namespace trustonic
}  // namespace vendor
